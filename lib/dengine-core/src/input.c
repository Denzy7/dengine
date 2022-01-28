#include "input.h"
#include "window.h" //glfw_current

#include <string.h> //memset

#include <stdio.h> // DEBUG

#ifdef DENGINE_WIN32
#include <windows.h>
#include <xinput.h>
#endif

#ifdef DENGINE_LINUX

#include <dirent.h> //opendir /dev/input
#include <linux/input.h>
#include <unistd.h>
#include <fcntl.h>

struct input_event play, stop, gain;
//2 basic rumble effects
struct ff_effect effects[2];
const char* linux_error_str = "";
//16 pads
typedef struct linux_pad
{
    int fd;  //fd from open()
    int hasrumble;  //test bit;
}linux_pad;

linux_pad linux_pads[16];
//Test a char bit
#define isBitSet(bit, arr) (arr[(bit) / 8] & (1 << ((bit) % 8)))

int _dengine_input_gamepad_linux_init();

int _dengine_input_gamepad_linux_vibrate(int pad, float left, float right);


int _dengine_input_gamepad_linux_init()
{
    //This returns 1 only when all effects have been successfully uploaded

    const char* dev = "/dev/input";

    DIR* dir = opendir(dev);
    char path[PATH_MAX];
    if(!dir)
        return 0;

    struct dirent* entry;
    int count = 0;
    memset(linux_pads, -1, sizeof(linux_pads));
    while((entry = readdir(dir)))
    {
        //only load eventXX
        if(!strncmp("event", entry->d_name, strlen("event")))
        {
            snprintf(path, sizeof(path), "%s/%s", dev, entry->d_name);
            int fd = open(path, O_RDWR );

            if(fd != -1)
            {
                linux_pads[count].fd = fd;
                char ffFeatures[(FF_CNT + 7) / 8] = {0};

                int fx = 0;
                if(ioctl(linux_pads[count].fd, EVIOCGEFFECTS, &fx) == -1)
                {
                    linux_error_str = "Cannot query number of effects";
                }

                //Load effects
                memset(ffFeatures, 0, sizeof(ffFeatures));
                if (ioctl(linux_pads[count].fd, EVIOCGBIT(EV_FF, sizeof(ffFeatures)), ffFeatures) == -1) {
                    linux_error_str = "Cannot query force feedback effects";
                    return 0;
                }

                //There are many tricks the Linux driver can do (Sine, Spring, Damp, etc...)
                //We just use a simple rumble

                //Do we have a rumble effect?
                if(isBitSet(FF_RUMBLE, ffFeatures))
                {
                    linux_pads[count].hasrumble = 1;

                    //Now upload strong rumble (left motor)
                    effects[0].type = FF_RUMBLE;
                    effects[0].id = -1;
                    effects[0].u.rumble.strong_magnitude = 0xFFFF;
                    effects[0].u.rumble.weak_magnitude = 0;
                    effects[0].replay.length = 2000;
                    effects[0].replay.delay = 0;
                    if (ioctl(linux_pads[count].fd, EVIOCSFF, &effects[0]) == -1) {
                        linux_error_str = "Cannot upload rumble to left motor";
                        return 0;
                    }

                    //Upload weak rumble, right motor
                    effects[1].type = FF_RUMBLE;
                    effects[1].id = -1;
                    effects[1].u.rumble.strong_magnitude = 0;
                    effects[1].u.rumble.weak_magnitude = 0xFFFF;
                    effects[1].replay.length = 2000;
                    effects[1].replay.delay = 0;
                    if (ioctl(linux_pads[count].fd, EVIOCSFF, &effects[1]) == -1) {
                        linux_error_str = "Cannot upload rumble to right motor";
                        return 0;
                    }
                }

                /* Set master gain to 75% if supported */
                if (isBitSet(FF_GAIN, ffFeatures)) {
                    memset(&gain, 0, sizeof(gain));
                    gain.type = EV_FF;
                    gain.code = FF_GAIN;
                    gain.value = 0xC000; /* [0, 0xFFFF]) */

                    if (write(linux_pads[count].fd, &gain, sizeof(gain)) != sizeof(gain)) {
                      linux_error_str = "Failed to upload gain";
                    }
                }
                count++;
            }
        }
    }
    closedir(dir);
    printf("fetch %d pads\n", count);

    return 1;
}

int _dengine_input_gamepad_linux_vibrate(int pad, float left, float right)
{
    if(linux_pads[pad].hasrumble == 1 && linux_pads[pad].fd != -1)
    {
        //Update effects
        effects[0].u.rumble.strong_magnitude = 0xFFFF * left;
        if (ioctl(linux_pads[pad].fd, EVIOCSFF, &effects[0]) == -1) {
            linux_error_str = "Cannot upload left rumble";
        }

        effects[1].u.rumble.weak_magnitude = 0xFFFF * right;
        if (ioctl(linux_pads[pad].fd, EVIOCSFF, &effects[1]) == -1) {
            linux_error_str = "Cannot upload right rumble";
        }

        //Play effects
        memset(&play,0,sizeof(play));
        play.type = EV_FF;
        play.code = effects[1].id;
        play.value = 1;

        if (write(linux_pads[pad].fd, (const void*) &play, sizeof(play)) == -1) {
            linux_error_str = "Cannot play left motor";
        }

        memset(&play,0,sizeof(play));
        play.type = EV_FF;
        play.code = effects[0].id;
        play.value = 1;

        if (write(linux_pads[pad].fd, (const void*) &play, sizeof(play)) == -1) {
            linux_error_str = "Cannot play right motor";
            return 0;
        }

        return 1;
    }else
    {
        linux_error_str = "Disconnected or has no rumble support";
        return 0;
    }
}

#endif

typedef struct key
{
    int key;
    int scancode;
}key;

#define DENGINE_INPUT_MAXKEYPRESS 10
#define DENGINE_INPUT_MAXMOUSEBTN 3
key keys[DENGINE_INPUT_MAXKEYPRESS];
int mousebtns[3] = {-1, -1, -1};
double mousescrollx = 0.0, mousescrolly = 0.0;
double mouseposx = 0.0,mouseposy = 0.0;
int padjid[16];

void _dengine_input_gamepad_validate();

void dengine_input_init()
{
#ifdef DENGINE_WIN_GLFW
    GLFWwindow* current = dengine_window_glfw_get_currentwindow();
    if(current)
    {
        glfwSetKeyCallback(current, dengine_input_glfw_callback_key);
        glfwSetMouseButtonCallback(current, dengine_input_glfw_callback_mousebtn);
        glfwSetScrollCallback(current, dengine_input_glfw_callback_mousescroll);
        glfwSetCursorPosCallback(current, dengine_input_glfw_callback_mousepos);
    }
    glfwSetJoystickCallback(dengine_input_glfw_callback_joystick);

    _dengine_input_gamepad_validate();

#endif
}


int dengine_input_get_key_once(int key)
{
    for(int i = 0; i < DENGINE_INPUT_MAXKEYPRESS; i++)
    {
        if(keys[i].key == key)
        {
            keys[i].scancode = 0;
            keys[i].key = 0;
            return 1;
        }
    }
    return 0;
}

int dengine_input_get_key(int key)
{
    for(int i = 0; i < DENGINE_INPUT_MAXKEYPRESS; i++)
    {
        if(keys[i].key == key)
        {
            return 1;
        }
    }
    return 0;
}

int dengine_input_get_mousebtn_once(int btn)
{
    for(int i = 0; i < DENGINE_INPUT_MAXMOUSEBTN; i++)
    {
        if(mousebtns[i] == btn)
        {
            mousebtns[i] = -1;
            return 1;
        }
    }
    return 0;
}

int dengine_input_get_mousebtn(int btn)
{
    for(int i = 0; i < DENGINE_INPUT_MAXMOUSEBTN; i++)
    {
        if(mousebtns[i] == btn)
        {
            return 1;
        }
    }
    return 0;
}

double dengine_input_get_mousescroll_x()
{
    double temp = mousescrollx;
    mousescrollx = 0.0;
    return temp;
}

double dengine_input_get_mousescroll_y()
{
    double temp = mousescrolly;
    mousescrolly = 0.0;
    return temp;
}

double dengine_input_get_mousepos_x()
{
    return mouseposx;
}

double dengine_input_get_mousepos_y()
{
    return mouseposy;
}

//GLFW callbacks
#ifdef DENGINE_WIN_GLFW
void dengine_input_glfw_callback_key(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    for(int i = 0; i < DENGINE_INPUT_MAXKEYPRESS; i++)
    {
        if(keys[i].scancode == scancode && action == GLFW_RELEASE)
        {
            keys[i].scancode = 0;
            keys[i].key = 0;
            //printf("rm key : %c, code : %d, action : %d, mods : %d\n", key, scancode, action, mods);
            break;
        }

        //don't dupe
        if(keys[i].scancode == scancode && action > 0)
            break;

        if(keys[i].scancode != scancode && action == GLFW_PRESS)
        {
            //printf("add key : %c, code : %d, action : %d, mods : %d\n", key, scancode, action, mods);
            keys[i].scancode = scancode;
            keys[i].key = key;
            break;
        }
    }
}

void dengine_input_glfw_callback_mousebtn(GLFWwindow* window, int button, int action, int mods)
{
    for(int i = 0; i < DENGINE_INPUT_MAXMOUSEBTN; i++)
    {
        if(mousebtns[i] == button && action == GLFW_RELEASE)
        {
            mousebtns[i] = -1;
            //printf("rm mousebtn : %d, action : %d, mods : %d\n", button, action, mods);
            break;
        }

        //don't dupe
        if(mousebtns[i] == button && action > 0)
            break;

        if(mousebtns[i] != button && action == GLFW_PRESS)
        {
            //printf("add mousebtn : %d, action : %d, mods : %d\n", button, action, mods);
            mousebtns[i] = button;
            break;
        }
    }
}

void dengine_input_glfw_callback_mousescroll(GLFWwindow* window, double x, double y)
{
    mousescrollx = x;
    mousescrolly = y;

    //printf("mousescroll, x : %f, y : %f\n", x, y);
}

void dengine_input_glfw_callback_mousepos(GLFWwindow* window, double x, double y)
{
    mouseposx = x;
    mouseposy = y;
    //printf("mousepos, x : %f, y : %f\n", x, y);
}

void dengine_input_glfw_callback_joystick(int jid, int event)
{
    _dengine_input_gamepad_validate();
}

#endif

void _dengine_input_gamepad_validate()
{
    memset(padjid, -1, sizeof(padjid));
    //Validate connected gamepads
    for(int i = 0; i < 16; i++)
    {

        #ifdef DENGINE_WIN_GLFW
        if(glfwJoystickIsGamepad(i))
        {
            padjid[i] = i;
        }
        #endif

        #ifdef DENGINE_LINUX
        if(linux_pads[i].fd != -1)
        {
            close(linux_pads[i].fd);
            linux_pads[i].fd = -1;
        }
        #endif
    }

    #ifdef DENGINE_LINUX
    _dengine_input_gamepad_linux_init();
    #endif
}

int dengine_input_gamepad_get_btn(int pad, int btn)
{
    #ifdef DENGINE_WIN_GLFW
    GLFWgamepadstate state;
    if(glfwGetGamepadState(pad, &state) && btn < 15)
        return state.buttons[btn];
    else
        return 0;
    #endif
}

float dengine_input_gamepad_get_axis(int pad, int axis)
{
    #ifdef DENGINE_WIN_GLFW
    GLFWgamepadstate state;
    if(glfwGetGamepadState(pad, &state) && axis < 6)
        return (state.axes[axis] + 1.0f) / 2.0f;
    else
        return 0.0f;
    #endif
}

int dengine_input_gamepad_get_isconnected(int pad)
{
    #ifdef DENGINE_WIN_GLFW
    return glfwJoystickIsGamepad(pad);
    #endif
}

const char* dengine_input_gamepad_get_name(int pad)
{
    #ifdef DENGINE_WIN_GLFW
    return glfwGetJoystickName(pad);
    #endif
}

int dengine_input_gamepad_vibration_set_basic(int pad, float leftmotor, float rightmotor)
{
    #if defined(DENGINE_WIN32)
    // Create a Vibraton State
    XINPUT_VIBRATION Vibration;
    ZeroMemory(&Vibration, sizeof(XINPUT_VIBRATION));

    Vibration.wLeftMotorSpeed = 0xFFFF * leftmotor;
    Vibration.wRightMotorSpeed = 0xFFFF * rightmotor;

    return XInputSetState(pad, &Vibration) == ERROR_SUCCESS ? 1 : 0;

    #elif defined(DENGINE_LINUX)
    return _dengine_input_gamepad_linux_vibrate(pad, leftmotor, rightmotor);
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
