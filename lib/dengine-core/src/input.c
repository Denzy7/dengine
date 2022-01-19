#include "input.h"
#include "window.h" //glfw_current

typedef struct _KeyState
{
    int pressed;
    int key;
}_KeyState;

#define DENGINE_INPUT_MAXKEYPRESS 4
_KeyState keys[DENGINE_INPUT_MAXKEYPRESS];

void dengine_input_init()
{
#ifdef DENGINE_WIN_GLFW
    GLFWwindow* current = dengine_window_glfw_get_currentwindow();
    glfwSetKeyCallback(current, dengine_input_glfw_keycallback);
#endif
}

//GLFW callbacks
#ifdef DENGINE_WIN_GLFW
void dengine_input_glfw_keycallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    //Check for released keys, set it to this one
    //Don't dupe
    for(int i = 0; i < DENGINE_INPUT_MAXKEYPRESS; i++)
    {
        //continue if we have the key
        if(keys[i].key == key)
            continue;

        if(!keys[i].pressed && keys[i].key != key)
        {
            keys[i].key = key;
            keys[i].pressed = 1;
            break;
        }
    }
}
#endif
