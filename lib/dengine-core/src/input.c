#include "dengine/input.h"
#include "dengine-core_internal.h"

#include "dengine-utils/debug.h"
#include "dengine-utils/macros.h"
#include "dengine-utils/logging.h"

#include <stdlib.h> /* abs */
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
#include <sys/inotify.h>                
#include <unistd.h> //O_RDWR
#include <fcntl.h> //write, open
//Test a char array bit
#define isBitSet(bit, arr) (arr[(bit) / 8] & (1 << ((bit) % 8)))
#endif

#ifdef DENGINE_LINUX
void _dengine_input_gamepad_linux_resetpads();
int _dengine_input_gamepad_linux_getpadfds();
int _dengine_input_gamepad_linux_play(GamepadID pad);
#endif

#define DENGINE_INPUT_GAMEPAD_MAXNAME 256
typedef struct
{
#ifdef DENGINE_LINUX
    int fd; //from open
    struct input_absinfo absinfo[DENGINE_INPUT_PAD_AXIS_COUNT];
#endif
#ifdef DENGINE_WIN32
   XINPUT_STATE state;
#endif

    char name[DENGINE_INPUT_GAMEPAD_MAXNAME];
    char connected;
    char hasrumble; //rumble support

    ButtonInfo buttons[DENGINE_INPUT_PAD_BUTTON_COUNT];
    float axes[DENGINE_INPUT_PAD_AXIS_COUNT];
}Gamepad;

#ifdef DENGINE_LINUX
struct ff_effect rumble;
static const char* const dev_input = "/dev/input";
int inotify_fd;
const char* linux_error_str = "";

/* dev and dir */

/* map similar GamepadAxis for read. but we'd use a user provided map
 * but i think jstest does this? */
static const uint32_t axismap[DENGINE_INPUT_PAD_AXIS_COUNT]=
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
static const uint32_t btnmap[DENGINE_INPUT_PAD_BUTTON_COUNT]=
{
    BTN_DPAD_UP,
    BTN_DPAD_DOWN,
    BTN_DPAD_LEFT,
    BTN_DPAD_RIGHT,
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

StandardInput* _stdinput = NULL;

void _dengine_input_init()
{
#ifdef DENGINE_LINUX
    if((inotify_fd = inotify_init1(IN_NONBLOCK)) != -1)
    {
        /*we need IN_ATTRIB coz kernel gives us IN_CREATE waaay before 
         * permission has been set. linux <3 
         */
        inotify_add_watch(inotify_fd, dev_input, IN_CREATE | IN_DELETE | IN_ATTRIB);
    }
    _dengine_input_gamepad_linux_getpadfds();
#endif
}

void _dengine_input_terminate()
{
#ifdef DENGINE_LINUX
    close(inotify_fd);
    /*_dengine_input_gamepad_linux_resetpads();*/
#endif
}

int _dengine_input_button_op(ButtonInfo* infos, size_t n, uint32_t button)
{
    for(size_t i = 0; i < n; i++)
    {
        if(infos[i].button == button && infos[i].state)
            return 1;
    }
    return 0;
}

int _dengine_input_button_oneshot_op(ButtonInfo* infos, size_t n, uint32_t button)
{
    for(size_t i = 0; i < n; i++)
    {
        if(infos[i].button == button && infos[i].oneshot)
            return 0;

        if(infos[i].button == button){
            infos[i].oneshot = 1;
            return 1;
        }
    }
    return 0;
}
void dengine_input_set_input(StandardInput* input)
{
    _stdinput = input;
}
int dengine_input_get_key_once(uint32_t key)
{
    DENGINE_DEBUG_ENTER;
    if(_stdinput == NULL)
        return 0;

    return _dengine_input_button_oneshot_op(_stdinput->keys, DENGINE_ARY_SZ(_stdinput->keys), key);
}

int dengine_input_get_key(uint32_t key)
{
    DENGINE_DEBUG_ENTER;

    if(_stdinput == NULL)
        return 0;

    return _dengine_input_button_op(_stdinput->keys, DENGINE_ARY_SZ(_stdinput->keys), key);
}

int dengine_input_get_mousebtn_once(MouseButton btn)
{
    DENGINE_DEBUG_ENTER;
    if(_stdinput == NULL)
        return 0;

    return _dengine_input_button_oneshot_op(_stdinput->mouse_btns, DENGINE_ARY_SZ(_stdinput->mouse_btns), btn);
}

int dengine_input_get_mousebtn(MouseButton btn)
{
    DENGINE_DEBUG_ENTER;

    if(_stdinput == NULL)
        return 0;

    return _dengine_input_button_op(_stdinput->mouse_btns, DENGINE_ARY_SZ(_stdinput->mouse_btns), btn);
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
    DENGINE_DEBUG_ENTER;

    if(_stdinput == NULL)
        return 0;

    double v = _stdinput->mouse_scroll_y;
    _stdinput->mouse_scroll_y = 0;
    return v;
}

double dengine_input_get_mousepos_x()
{
    DENGINE_DEBUG_ENTER;

    if(_stdinput == NULL)
        return 0;

    return _stdinput->mouse_x;
}

double dengine_input_get_mousepos_y()
{
    DENGINE_DEBUG_ENTER;

    if(_stdinput == NULL)
        return 0;

    return _stdinput->mouse_y;
}

int dengine_input_gamepad_get_btn(GamepadID pad, GamepadButton btn)
{
    DENGINE_DEBUG_ENTER;

    return _dengine_input_button_op(_gamepads[pad].buttons, DENGINE_ARY_SZ(_gamepads[pad].buttons), btn);
}

int dengine_input_gamepad_get_btn_once(GamepadID pad, GamepadButton btn)
{
    DENGINE_DEBUG_ENTER;

    return _dengine_input_button_oneshot_op(_gamepads[pad].buttons, DENGINE_ARY_SZ(_gamepads[pad].buttons), btn);
}

float dengine_input_gamepad_get_axis(GamepadID pad, GamepadAxis axis)
{
    DENGINE_DEBUG_ENTER;
    return _gamepads[pad].axes[axis];
}

int dengine_input_gamepad_get_isconnected(GamepadID pad)
{
    DENGINE_DEBUG_ENTER;
    //dengine_input_gamepad_poll();
    return _gamepads[pad].connected;
}

const char* dengine_input_gamepad_get_name(GamepadID pad)
{
    DENGINE_DEBUG_ENTER;

    return _gamepads[pad].name;
}

int dengine_input_gamepad_vibration_set_basic(GamepadID pad, float leftmotor, float rightmotor)
{
    DENGINE_DEBUG_ENTER;

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
int _dengine_input_get_touch_inregion(int x, int y, int width, int height, const SoftInput* touch)
{
    return 
        touch->x >= x &&
        touch->x <= (x + width) &&
        touch->y >= y &&
        touch->y <= (y + height);
}
int dengine_input_get_touch(int x, int y, int width, int height)
{
    for(size_t i = 0; i < DENGINE_ARY_SZ(_stdinput->touches); i++)
    {
        int inregion = _dengine_input_get_touch_inregion(x, y, width, height, &_stdinput->touches[i]);
        if(inregion && _stdinput->touches[i].isdown)
            return 1;
    }
    return 0;
}

int dengine_input_get_touch_once(int x, int y, int width, int height)
{
    for(size_t i = 0; i < DENGINE_ARY_SZ(_stdinput->touches); i++)
    {
        int inregion = _dengine_input_get_touch_inregion(x, y, width, height, &_stdinput->touches[i]);
           
        if(inregion && _stdinput->touches[i].oneshot)
            return 0;

        if(inregion && _stdinput->touches[i].isdown)
        {
            _stdinput->touches[i].oneshot = 1;
            return 1;
        }
    }

    return 0;
}

int _dengine_input_gamepad_poll()
{
#if defined(DENGINE_LINUX)

    char evbuffer[sizeof(struct inotify_event) + NAME_MAX + 1];
    struct inotify_event* ev;

    do {
        if(read(inotify_fd, evbuffer, sizeof(evbuffer)) == -1)
            break;
        ev = (struct inotify_event*)evbuffer;
        if(ev->mask & IN_CREATE || ev->mask & IN_DELETE || ev->mask & IN_ATTRIB)
            _dengine_input_gamepad_linux_getpadfds();
    }while ((void*)ev + ev->len != NULL);

    /* Check gamepads */
    for(int i = 0; i < DENGINE_INPUT_PAD_COUNT; i++)
    {
        if(!_gamepads[i].connected)
            continue;

        struct input_event ev;
        if(read(_gamepads[i].fd, &ev, sizeof(ev)) == -1)
            continue;

        if(ev.type == EV_ABS)
        {
            for(int j = 0; j < DENGINE_INPUT_PAD_AXIS_COUNT; j++)
            {
                if(ev.code == axismap[j])
                {
                    int lim = _gamepads[i].absinfo[j].maximum - _gamepads[i].absinfo[j].minimum;
                    float clamp = (float)ev.value / (float)lim;
                    _gamepads[i].axes[j] = (clamp * 2.0) - 1.0;
                }
            }
            /*TODO: some dumb controllers report the dpad
             * as an axis.
             */
/*            static const int32_t dpadcodes[8]=*/
            /*{*/
                /*ABS_HAT0Y, -1,*/
                /*ABS_HAT0Y, 1,*/
                /*ABS_HAT0X, -1,*/
                /*ABS_HAT0X, 1,*/
            /*};*/
            /*for(int j = 0; j < 4; j++)*/
            /*{*/
                /*if(dpadcodes[(2 * j) + 1] == ev.value && dpadcodes[2 * j] == ev.code)*/
                /*{*/
                /*}*/
            /*}*/
        }else if(ev.type == EV_KEY)
        {
            for(int j = 0; j < DENGINE_INPUT_PAD_BUTTON_COUNT; j++)
            {
                if(btnmap[j] == ev.code)
                {
                    _gamepads[i].buttons[j].button = j;
                    _gamepads[i].buttons[j].state = ev.value;
                    if(ev.value == 0)
                        memset(&_gamepads[i].buttons[j], 0, sizeof(ButtonInfo));
                }
            }
        }
    }
    return 1;
#elif defined(DENGINE_WIN32)
    for(int i = 0; i < DENGINE_INPUT_PAD_COUNT; i++)
    {
        /*TODO: dinput has better flexibility :) */
        if(XInputGetState(i, &_gamepads[i].state) == ERROR_SUCCESS)
        {
            _gamepads[i].connected = 1;
            /* will do for now! */
            snprintf(_gamepads[i].name, sizeof(_gamepads[i].name), "XINPUT CONTROLLER %d", i);

            for(int j = 0; j < DENGINE_INPUT_PAD_BUTTON_COUNT; j++)
            {
                if(_gamepads[i].state.Gamepad.wButtons & btnbits[j])
                {
                    _gamepads[i].buttons[j].button = j;
                    _gamepads[i].buttons[j].state = 1;
                }else
                {
                    memset(&_gamepads[i].buttons[j], 0, sizeof(ButtonInfo));
                }
            }

            _gamepads[i].axes[DENGINE_INPUT_PAD_AXIS_LT] = _gamepads->state.Gamepad.bLeftTrigger / 255.0f;
            _gamepads[i].axes[DENGINE_INPUT_PAD_AXIS_RT] = _gamepads->state.Gamepad.bRightTrigger / 255.0f;

            _gamepads[i].axes[DENGINE_INPUT_PAD_AXIS_LX] = _gamepads->state.Gamepad.sThumbLX / 255.0f;
            _gamepads[i].axes[DENGINE_INPUT_PAD_AXIS_LY] = _gamepads->state.Gamepad.sThumbLY / 255.0f;

            _gamepads[i].axes[DENGINE_INPUT_PAD_AXIS_RX] = _gamepads->state.Gamepad.sThumbRX / 255.0f;
            _gamepads[i].axes[DENGINE_INPUT_PAD_AXIS_RY] = _gamepads->state.Gamepad.sThumbRY / 255.0f;
        }
    }
    return 1;
#endif
}

#ifdef DENGINE_LINUX
void _dengine_input_gamepad_linux_resetpads()
{
    for(size_t i = 0; i < DENGINE_ARY_SZ(_gamepads); i++)
    {
        close(_gamepads[i].fd);
        if(_gamepads[i].connected)
            dengineutils_logging_log("WARNING::Controller [ %s ] disconnected", _gamepads[i].name);
        memset(&_gamepads[i], 0, sizeof(Gamepad));
    }
}
int _dengine_input_gamepad_linux_getpadfds()
{
    uint32_t axes = 0;
    DIR* dir;
    int count = 0;
    char path[PATH_MAX];
    struct input_event gain;
    char ff;

    _dengine_input_gamepad_linux_resetpads();

    if((dir = opendir(dev_input)) == NULL)
    {
        dengineutils_logging_log("ERROR::cannot open %s", dev_input);
        return 0;
    }

    struct dirent* ent;

    while((ent = readdir(dir)) && count < DENGINE_INPUT_PAD_COUNT)
    {
        axes = 0;

        if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") || strncmp(ent->d_name, "event", 5))
            continue;

        snprintf(path, sizeof(path), "%s/%s", dev_input, ent->d_name);

        if((_gamepads[count].fd = open(path, O_RDWR | O_NONBLOCK)) == -1){
            continue;
        }

        if(ioctl(_gamepads[count].fd, EVIOCGNAME(DENGINE_INPUT_GAMEPAD_MAXNAME),
                    _gamepads[count].name) == -1)
            continue;

        for(size_t i = 0; i < DENGINE_INPUT_PAD_AXIS_COUNT; i++)
        {
            if(ioctl(_gamepads[count].fd, EVIOCGABS(axismap[i]), &_gamepads[count].absinfo[i]) == -1)
                break;
            else 
                axes++;
        }
        if(axes < DENGINE_ARY_SZ(axismap)){
            dengineutils_logging_log("WARNING::Controller [ %s ] does not have all axes needed. skipped... (have you mapped the axes? we need at least ABS_{Z,RZ,X,Y,RX,RY} mapped with jstest-gtk)");
            continue;
        }

        if(ioctl(_gamepads[count].fd, EVIOCGBIT(EV_FF, sizeof(ff)), &ff) != -1)
        {
            /* There are many tricks the Linux driver can do (Sine, Spring, Damp, etc...)
            We just use a simple rumble */

            /*TODO: untested. UNTIL I GET MY HANDS ON A CONTROLLER! */
            if(ff & FF_RUMBLE)
            {

                memset(&rumble, 0, sizeof(rumble));
                /* Now upload rumble */
                rumble.type = FF_RUMBLE;
                rumble.id = -1;
                rumble.replay.length = 100;
                if (ioctl(_gamepads[count].fd, EVIOCSFF, &rumble) != -1) {
                    _gamepads[count].hasrumble = 1;
                }
            }
            if(ff & FF_GAIN)
            {
                memset(&gain, 0, sizeof(gain));
                gain.type = EV_FF;
                gain.code = FF_GAIN;
                gain.value = 0xC000; /* [0, 0xFFFF]) */
                if (write(_gamepads[count].fd, &gain, sizeof(gain)) != sizeof(gain)) {
                    perror("gain effect upload failed");
                }
            }
        }else {
            perror("force feedback");
        }

        dengineutils_logging_log("INFO::New controller [ %s ] axes : %d, rumble_force_feedback : %d", _gamepads[count].name, axes, _gamepads[count].hasrumble);

        _gamepads[count].connected = 1;
        count++;
    }

    closedir(dir);

    return count;
}


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

    if (write(_gamepads[pad].fd, (const void*) &event, sizeof(event)) == -1) {
        linux_error_str = "Cannot play effect";
        return 0;
    }

    return 1;
}
#endif
