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

int dengine_input_get_key_once(int key);

int dengine_input_get_key(int key);

int dengine_input_get_mousebtn_once(int btn);

int dengine_input_get_mousebtn(int btn);

double dengine_input_get_mousescroll_x();

double dengine_input_get_mousescroll_y();

double dengine_input_get_mousepos_x();

double dengine_input_get_mousepos_y();

const char* dengine_input_gamepad_get_name(int pad);

int dengine_input_gamepad_get_isconnected(int pad);

int dengine_input_gamepad_get_btn(int pad, int btn);

float dengine_input_gamepad_get_axis(int pad, int axis);

//GLFW callbacks
#ifdef DENGINE_WIN_GLFW

void dengine_input_glfw_callback_key(GLFWwindow* window,int key,int scancode,int action,int mods);

void dengine_input_glfw_callback_mousebtn(GLFWwindow* window, int button, int action, int mods);

void dengine_input_glfw_callback_mousescroll(GLFWwindow* window, double x, double y);

void dengine_input_glfw_callback_mousepos(GLFWwindow* window, double x, double y);

void dengine_input_glfw_callback_joystick(int jid, int event);

#endif



#ifdef __cplusplus
}
#endif

#endif // INPUT_H
