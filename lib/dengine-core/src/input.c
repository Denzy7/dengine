#include "input.h"
#include "window.h" //glfw_current

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
int padjid = 0;
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
    if(event == GLFW_CONNECTED)
    {
        //printf("Connected : %s\n", glfwGetJoystickName(jid));
        padjid = jid;
    }else
    {
        //printf("Disconnected : %s\n", glfwGetJoystickName(jid));
    }
}

#endif
