#include "dengine/input.h"

#include "dengine-utils/os.h" //dir_filecount
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
#include <unistd.h> //O_RDWR
#include <fcntl.h> //write, open

//Test a char array bit
#define isBitSet(bit, arr) (arr[(bit) / 8] & (1 << ((bit) % 8)))
#endif

#ifdef DENGINE_LINUX
int _dengine_input_gamepad_linux_play(GamepadID pad, const struct ff_effect effect);
#endif

struct Gamepad
{
#ifdef DENGINE_LINUX
    int fd; //from open
#endif
    char connected;
    char hasrumble; //rumble support
};

#ifdef DENGINE_LINUX
int lastcount = 0; //track last count in /dev/input/eventXX
struct ff_effect rumble;
const char* linux_error_str = "";
#endif

Gamepad _gamepads[DENGINE_INPUT_PAD_COUNT];

WindowInput* _windowinp;

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
    return 0;
}

float dengine_input_gamepad_get_axis(GamepadID pad, GamepadAxis axis)
{
    return 0.0f;
}

int dengine_input_gamepad_get_isconnected(GamepadID pad)
{
    return 0;
}

const char* dengine_input_gamepad_get_name(GamepadID pad)
{
    return NULL;
}

int dengine_input_gamepad_vibration_set_basic(GamepadID pad, float leftmotor, float rightmotor)
{
    #if defined(DENGINE_WIN32)
    // Create a Vibraton State
    XINPUT_VIBRATION Vibration;
    ZeroMemory(&Vibration, sizeof(XINPUT_VIBRATION));

    Vibration.wLeftMotorSpeed = 0xFFFF * leftmotor;
    Vibration.wRightMotorSpeed = 0xFFFF * rightmotor;

    return XInputSetState(pad, &Vibration) == ERROR_SUCCESS ? 1 : 0;

    #elif defined(DENGINE_LINUX)
    rumble.u.rumble.strong_magnitude = 0xFFFF * leftmotor;
    rumble.u.rumble.weak_magnitude = 0xFFFF * rightmotor;
    if(_dengine_input_gamepad_linux_play(pad, rumble) &&
            _dengine_input_gamepad_linux_play(pad, rumble))
    {
        return 1;
    }else
    {
        return 0;
    }
    #else
    return 0;
    #endif
}

const char* dengine_input_gamepad_vibration_get_error()
{
    #if defined(DENGINE_LINUX)
    return linux_error_str;
    #else
    return "";
    #endif
}

int dengine_input_gamepad_poll()
{
#ifdef DENGINE_LINUX
    //This returns 1 only when all effects have been successfully uploaded

    static const char* dev = "/dev/input";
    int current = dengineutils_os_dir_filecount(dev);
    if(current == lastcount)
        return 0;

    DIR* dir = opendir(dev);
    char path[PATH_MAX];
    if(!dir)
        return 0;

    struct dirent* entry;
    int count = 0;

    memset(&_gamepads, 0, sizeof(_gamepads));
    while((entry = readdir(dir)) && count < DENGINE_INPUT_PAD_COUNT)
    {
        //only load eventXX
        if(!strcmp("event", entry->d_name))
        {
            snprintf(path, sizeof(path), "%s/%s", dev, entry->d_name);
            _gamepads[count].fd = open(path, O_RDWR );

            if(_gamepads[count].fd != -1)
            {
                char ffFeatures[(FF_CNT + 7) / 8] = {0};

                int fx = 0;
                if(ioctl(_gamepads[count].fd, EVIOCGEFFECTS, &fx) == -1)
                {
                    linux_error_str = "Cannot query number of effects";
                }

                //Load effects
                memset(ffFeatures, 0, sizeof(ffFeatures));
                if (ioctl(_gamepads[count].fd, EVIOCGBIT(EV_FF, sizeof(ffFeatures)), ffFeatures) == -1) {
                    linux_error_str = "Cannot query force feedback effects";
                    return 0;
                }

                //There are many tricks the Linux driver can do (Sine, Spring, Damp, etc...)
                //We just use a simple rumble

                //Do we have a rumble effect?
                if(isBitSet(FF_RUMBLE, ffFeatures))
                {
                    _gamepads[count].hasrumble = 1;

                    memset(&rumble, 0, sizeof(rumble));
                    //Now upload rumble
                    rumble.type = FF_RUMBLE;
                    rumble.id = -1;
                    rumble.replay.length = 2000;
                    if (ioctl(_gamepads[count].fd, EVIOCSFF, &rumble) == -1) {
                        linux_error_str = "Cannot upload rumble";
                        return 0;
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
                count++;
            }
        }
    }
    lastcount = count;
    closedir(dir);
    return 1;
#endif
}

#ifdef DENGINE_LINUX
int _dengine_input_gamepad_linux_play(GamepadID pad, const struct ff_effect effect)
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
    event.code = effect.id;
    event.value = 1;
    if (ioctl(_gamepads[pad].fd, EVIOCSFF, &effect) == -1) {
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
