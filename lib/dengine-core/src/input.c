#include "dengine/input.h"
#include "dengine-core_internal.h"

#include <string.h> //memset
#include <stdio.h> // snprintf

#ifdef DENGINE_ANDROID
#include "dengine-utils/platform/android.h"
#endif

#ifdef DENGINE_WIN32
#include <windows.h>
#include <xinput.h>
#endif

#ifdef DENGINE_LINUX
#include <dirent.h> //opendir /dev/input
#include <linux/input.h> //ff, ev
#include <sys/stat.h> //* fstatat */
#include <unistd.h> //O_RDWR
#include <fcntl.h> //write, open
#include <stdlib.h> /* abs */
//Test a char array bit
#define isBitSet(bit, arr) (arr[(bit) / 8] & (1 << ((bit) % 8)))
#endif

#ifdef DENGINE_LINUX
int _dengine_input_gamepad_linux_play(GamepadID pad);
#endif

typedef struct
{
    float value;
    int max;
    int min;
}AxisInfo;

#define DENGINE_INPUT_GAMEPAD_MAXNAME 256
typedef struct
{
#ifdef DENGINE_LINUX
    int fd; //from open
    char path[256]; /* for checking existing eventX */
#endif
#ifdef DENGINE_WIN32
   XINPUT_STATE state;
#endif
    char name[DENGINE_INPUT_GAMEPAD_MAXNAME];
    char connected;
    char hasrumble; //rumble support

    char buttons[DENGINE_INPUT_PAD_BUTTON_COUNT];
    AxisInfo axes[DENGINE_INPUT_PAD_AXIS_COUNT];
}Gamepad;


#ifdef DENGINE_LINUX
struct ff_effect rumble;
const char* linux_error_str = "";

/* dev and dir */
static const char* dev = "/dev/input";
DIR* dir = NULL;

/* map similar GamepadAxis for read */
static const uint32_t axiscodes[DENGINE_INPUT_PAD_AXIS_COUNT]=
{
    ABS_Z,
    ABS_RZ,
    ABS_X,
    ABS_Y,
    ABS_RX,
    ABS_RY
};

/* map similar GamepadAxis for read
 DENGINE_INPUT_PAD_BUTTON_COUNT is not filled */
static const uint32_t btncodes[DENGINE_INPUT_PAD_BUTTON_COUNT]=
{
    DENGINE_INPUT_PAD_BUTTON_COUNT,
    DENGINE_INPUT_PAD_BUTTON_COUNT,
    DENGINE_INPUT_PAD_BUTTON_COUNT,
    DENGINE_INPUT_PAD_BUTTON_COUNT,
    BTN_START,
    BTN_BACK,
    BTN_THUMBL,
    BTN_THUMBR,
    BTN_TL,
    BTN_TR,
    BTN_A,
    BTN_B,
    BTN_X,
    BTN_Y
};
#endif
#ifdef DENGINE_WIN32
/* bit map similar GamepadAxis for read */
static const uint16_t btnbits[DENGINE_INPUT_PAD_BUTTON_COUNT]=
{
       0x1,
       0x2,
       0x4,
       0x8,
      0x10,
      0x20,
      0x40,
      0x80,
     0x100,
    0x0200,
    0x1000,
    0x2000,
    0x4000,
    0x8000
};
#endif


Gamepad _gamepads[DENGINE_INPUT_PAD_COUNT];

WindowInput* _windowinp;

void _dengine_input_init()
{
#ifdef DENGINE_LINUX
    dir = opendir(dev);
#endif
    memset(&_gamepads, 0, sizeof(_gamepads));
}

void _dengine_input_terminate()
{
#ifdef DENGINE_LINUX
    closedir(dir);
#endif
}

void dengine_input_set_window(DengineWindow* window)
{
    _windowinp = dengine_window_get_input(window);
}

int dengine_input_get_key_once(char key)
{
    for(int i = 0; i < DENGINE_WINDOW_ALPNUM; i++)
    {
        if(_windowinp->alpnum[i].key == key && _windowinp->alpnum[i].state != -1)
        {
            _windowinp->alpnum[i].state = -1;
            return 1;
        }
    }
    return 0;
}

int dengine_input_get_key(char key)
{
    for(int i = 0; i < DENGINE_WINDOW_ALPNUM; i++)
    {
        if(_windowinp->alpnum[i].key == key)
        {
            return 1;
        }
    }
    return 0;
}

int dengine_input_get_mousebtn_once(MouseButton btn)
{
    if(_windowinp->msebtn[btn] == 1)
    {
        _windowinp->msebtn[btn] = -1;
        return 1;
    }
    return 0;
}

int dengine_input_get_mousebtn(MouseButton btn)
{
    if(_windowinp->msebtn[btn] == 1)
    {
        return 1;
    }
    return 0;
}

//double dengine_input_get_mousescroll_x()
//{
//    double temp = mousescrollx;
//    mousescrollx = 0.0;
//    return temp;
//    return 0.0;
//}

double dengine_input_get_mousescroll_y()
{
    double temp = _windowinp->msesrl_y;
    _windowinp->msesrl_y = 0.0;
    return temp;
}

double dengine_input_get_mousepos_x()
{
    return _windowinp->mse_x;
}

double dengine_input_get_mousepos_y()
{
    return _windowinp->mse_y;
}

int dengine_input_gamepad_get_btn(GamepadID pad, GamepadButton btn)
{
    if(_gamepads[pad].buttons[btn] == 1)
        return 1;
    else
        return 0;
}

int dengine_input_gamepad_get_btn_once(GamepadID pad, GamepadButton btn)
{
    if(_gamepads[pad].buttons[btn] == 1)
    {
        _gamepads[pad].buttons[btn] = -1;
        return 1;
    }else
    {
        return 0;
    }
}

float dengine_input_gamepad_get_axis(GamepadID pad, GamepadAxis axis)
{
    return _gamepads[pad].axes[axis].value;
}

int dengine_input_gamepad_get_isconnected(GamepadID pad)
{
    //dengine_input_gamepad_poll();
    return _gamepads[pad].connected;
}

char* dengine_input_gamepad_get_name(GamepadID pad)
{
    return _gamepads[pad].name;
}

int dengine_input_gamepad_vibration_set_basic(GamepadID pad, float leftmotor, float rightmotor)
{
    /* block 0.0f repeated writes */
    float tot = leftmotor + rightmotor;
    if(tot == 0.0f)
        return 1;

    int ret = 0;

    #if defined(DENGINE_WIN32)
    // Create a Vibraton State
    XINPUT_VIBRATION Vibration;
    ZeroMemory(&Vibration, sizeof(XINPUT_VIBRATION));

    Vibration.wLeftMotorSpeed = 0xFFFF * leftmotor;
    Vibration.wRightMotorSpeed = 0xFFFF * rightmotor;

    ret = XInputSetState(pad, &Vibration) == ERROR_SUCCESS ? 1 : 0;

    #elif defined(DENGINE_LINUX)
    rumble.u.rumble.strong_magnitude = 0xFFFF * leftmotor;
    rumble.u.rumble.weak_magnitude = 0xFFFF * rightmotor;
    ret = _dengine_input_gamepad_linux_play(pad);
    #endif

    return ret;
}

const char* dengine_input_gamepad_vibration_get_error()
{
    #if defined(DENGINE_LINUX)
    return linux_error_str;
    #else
    return "";
    #endif
}

int _dengine_input_gamepad_poll()
{
#if defined(DENGINE_LINUX)
    #ifdef DENGINE_ANDROID_TERMUX
    /* block since /dev is unreadable */
    return 0;
    #endif

    struct dirent* entry;
    char path[PATH_MAX];
    int count = 0;

    /* Iterate to find valid gamepads */
    rewinddir(dir);
    while((entry = readdir(dir)) && count < DENGINE_INPUT_PAD_COUNT)
    {
        /* ignore if we already have it */
        int have = 0;
        for(int i = 0; i < DENGINE_INPUT_PAD_COUNT; i++)
        {
            if(strstr(_gamepads[i].path, entry->d_name))
                have = 1;
        }
        if(have)
            continue;

        /* only load eventX */
        if(strncmp("event", entry->d_name, 5) != 0)
            continue;

        /* skip if current is connected */
        if(_gamepads[count].connected && count < (DENGINE_INPUT_PAD_COUNT - 1))
        {
            count++;
        }

        /* out of memory! */
        if(count == DENGINE_INPUT_PAD_COUNT)
            continue;

        snprintf(path, sizeof(path), "%s/%s", dev, entry->d_name);
        _gamepads[count].fd = open(path, O_RDWR | O_NONBLOCK);

        /* work with only valid fd's */
        if(_gamepads[count].fd == -1)
            continue;

        strncpy(_gamepads[count].path, path, sizeof(_gamepads[count].path));

        /* get some axis info. if fail, prolly not a joystick */
        struct input_absinfo abs;
        for(int i = 0; i < DENGINE_INPUT_PAD_AXIS_COUNT; i++)
        {
            if(ioctl(_gamepads[count].fd, EVIOCGABS(axiscodes[i]), &abs) != -1)
            {
                _gamepads[count].axes[i].max = abs.maximum;
                _gamepads[count].axes[i].min = abs.minimum;
            }
        }

        /* check for force feedback (optional) */
        char ffFeatures[(FF_CNT + 7) / 8] = {0};

        int fx = 0;
        if(ioctl(_gamepads[count].fd, EVIOCGEFFECTS, &fx) == -1)
        {
            linux_error_str = "Cannot query number of effects";
        }else
        {
            /* Load effects */
            memset(ffFeatures, 0, sizeof(ffFeatures));
            if (ioctl(_gamepads[count].fd, EVIOCGBIT(EV_FF, sizeof(ffFeatures)), ffFeatures) == -1) {
                linux_error_str = "Cannot query force feedback effects";
            }

            /* There are many tricks the Linux driver can do (Sine, Spring, Damp, etc...)
            We just use a simple rumble */

            /* Do we have a rumble effect? */
            if(isBitSet(FF_RUMBLE, ffFeatures))
            {
                _gamepads[count].hasrumble = 1;

                memset(&rumble, 0, sizeof(rumble));
                /* Now upload rumble */
                rumble.type = FF_RUMBLE;
                rumble.id = -1;
                rumble.replay.length = 100;
                if (ioctl(_gamepads[count].fd, EVIOCSFF, &rumble) == -1) {
                    linux_error_str = "Cannot upload rumble";
                }
            }

            struct input_event event;
            /* Set master gain to 75% if supported */
            if (isBitSet(FF_GAIN, ffFeatures)) {
                memset(&event, 0, sizeof(event));
                event.type = EV_FF;
                event.code = FF_GAIN;
                event.value = 0xC000; /* [0, 0xFFFF]) */

                if (write(_gamepads[count].fd, &event, sizeof(event)) != sizeof(event)) {
                  linux_error_str = "Failed to upload gain";
                }
            }
        }

        /* get name. if error, prolly not gamepads! */
        if(ioctl(_gamepads[count].fd,
                 EVIOCGNAME(DENGINE_INPUT_GAMEPAD_MAXNAME),
                 _gamepads[count].name) != -1)
        {
            _gamepads[count].connected = 1;
//            dengineutils_logging_log("INFO::New Gamepad: %s fd : [%d]",_gamepads[count].name, _gamepads[count].fd);
        }

        count++;
    }

    /* Check gamepads */
    for(int i = 0; i < DENGINE_INPUT_PAD_COUNT; i++)
    {
        struct stat fstat;
        int ok = stat(_gamepads[i].path, &fstat);
        /* close valid fd's and ZeroMem it */
        if(ok != 0 && _gamepads[i].connected)
        {
//            dengineutils_logging_log("WARNING::Gamepad disconnect : %s fd:[%d]", _gamepads[i].name, _gamepads[i].fd);
            close(_gamepads[i].fd);
            memset(&_gamepads[i], 0, sizeof(Gamepad));
        }
        if(_gamepads[i].connected)
        {
            /* read input events */
            struct input_event ev;
            int rd = read(_gamepads[i].fd, &ev, sizeof(ev));
            if(rd == -1)
                continue;

            if(ev.type == EV_ABS)
            {
                for(int j = 0; j < DENGINE_INPUT_PAD_AXIS_COUNT; j++)
                {
                    if(axiscodes[j] == ev.code)
                    {
                        int lim = _gamepads[i].axes[j].max - _gamepads[i].axes[j].min;
                        float val = ev.value / (float)lim;
                        /* TODO: any better way? */
                        if( _gamepads[i].axes[j].min < 0.0f)
                            val *= 2.0f;
                        _gamepads[i].axes[j].value = val;
                    }
                }

                static const int32_t dpadcodes[8]=
                {
                    ABS_HAT0Y, -1,
                    ABS_HAT0Y, 1,
                    ABS_HAT0X, -1,
                    ABS_HAT0X, 1,
                };

                for(int j = 0; j < 4; j++)
                {
                    if(dpadcodes[(2 * j) + 1] == ev.value && dpadcodes[2 * j] == ev.code)
                    {
                        if(_gamepads[i].buttons[j] != -1)
                            _gamepads[i].buttons[j] = 1;
                    }else
                    {
                        _gamepads[i].buttons[j] = 0;
                    }
                }
                /* TODO: check dpad */
            }else if(ev.type == EV_KEY)
            {
                for(int j = 0; j < DENGINE_INPUT_PAD_BUTTON_COUNT; j++)
                {
                    if(btncodes[j] == ev.code)
                    {
                        if(ev.value && _gamepads[i].buttons[j] != -1){
                            _gamepads[i].buttons[j] = 1;
                        }else
                        {
                            _gamepads[i].buttons[j] = 0;
                        }
                    }
                }
            }
        }
    }
    return 1;
#elif defined(DENGINE_WIN32)
    for(int i = 0; i < DENGINE_INPUT_PAD_COUNT; i++)
    {
        if(XInputGetState(i, &_gamepads[i].state) == ERROR_SUCCESS)
        {
            _gamepads[i].connected = 1;
            /* will do for now! */
            snprintf(_gamepads[i].name, sizeof(_gamepads[i].name), "XINPUT CONTROLLER %d", i);

            for(int j = 0; j < DENGINE_INPUT_PAD_BUTTON_COUNT; j++)
            {
                if(_gamepads[i].state.Gamepad.wButtons & btnbits[j])
                {
                    if(_gamepads[i].buttons[j] != -1)
                        _gamepads[i].buttons[j] = 1;
                }else
                {
                    _gamepads[i].buttons[j] = 0;
                }
            }

            _gamepads[i].axes[DENGINE_INPUT_PAD_AXIS_LT].value = _gamepads->state.Gamepad.bLeftTrigger / 255.0f;
            _gamepads[i].axes[DENGINE_INPUT_PAD_AXIS_RT].value = _gamepads->state.Gamepad.bRightTrigger / 255.0f;

            _gamepads[i].axes[DENGINE_INPUT_PAD_AXIS_LX].value = _gamepads->state.Gamepad.sThumbLX / 255.0f;
            _gamepads[i].axes[DENGINE_INPUT_PAD_AXIS_LY].value = _gamepads->state.Gamepad.sThumbLY / 255.0f;

            _gamepads[i].axes[DENGINE_INPUT_PAD_AXIS_RX].value = _gamepads->state.Gamepad.sThumbRX / 255.0f;
            _gamepads[i].axes[DENGINE_INPUT_PAD_AXIS_RY].value = _gamepads->state.Gamepad.sThumbRY / 255.0f;
        }
    }
    return 1;
#endif
}

#ifdef DENGINE_LINUX
int _dengine_input_gamepad_linux_play(GamepadID pad)
{
    if(!_gamepads[pad].hasrumble)
    {
        linux_error_str = "No rumble support";
        return 0;
    }
    if(_gamepads[pad].fd == -1)
    {
        linux_error_str = "Disconnected";
        return 0;
    }

    struct input_event event;
    memset(&event, 0, sizeof(event));
    event.type = EV_FF;
    event.code = rumble.id;
    event.value = 1;

    if (ioctl(_gamepads[pad].fd, EVIOCSFF, &rumble) == -1) {
        linux_error_str = "Cannot upload effect";
        return 0;
    }

    if (write(_gamepads[pad].fd, (const void*) &event, sizeof(event)) == -1) {
        linux_error_str = "Cannot play effect";
        return 0;
    }

    return 1;
}
#endif
