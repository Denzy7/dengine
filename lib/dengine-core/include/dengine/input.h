#ifndef INPUT_H
#define INPUT_H

#include "dengine_config.h"

#ifdef DENGINE_WIN_GLFW
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

void dengine_input_init();

//GLFW callbacks
#ifdef DENGINE_WIN_GLFW

void dengine_input_glfw_keycallback(GLFWwindow* window,int key,int scancode,int action,int mods);

#endif



#ifdef __cplusplus
}
#endif

#endif // INPUT_H
