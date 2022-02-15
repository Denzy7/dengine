//USES ONLY ONE NATIVE WINDOW.

#ifndef WINDOW_H
#define WINDOW_H

#include "dengine_config.h" //DENGINE_WIN_, DENGINE_GL_

#if defined (DENGINE_WIN_GLFW)
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#elif defined (DENGINE_WIN_EGL)
#include <EGL/egl.h>
#else
#error "Plz define at least one Windowing Framework"
#endif // defined

#ifdef DENGINE_ANDROID
#include <android/window.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int dengine_window_init();

void dengine_window_terminate();

void dengine_window_request_GL(int gl_major, int gl_minor, int gl_core);

void dengine_window_request_MSAA(int samples);

void dengine_window_get_window_width(int* width);

void dengine_window_get_window_height(int* height);

void dengine_window_get_window_dim(int* width, int* height);

void dengine_window_set_viewport(int width, int height);

void dengine_window_swapbuffers();

int dengine_window_isrunning();

int dengine_window_loadgl();

//GLFW Specific Calls
#if defined(DENGINE_WIN_GLFW)

//callbacks


void dengine_window_glfw_callback_error(int code, const char* description);

void dengine_window_glfw_callback_fbsize(GLFWwindow* window, int width, int height);

void dengine_window_glfw_callback_windowclose(GLFWwindow* window);

int dengine_window_glfw_create(int width, int height, const char* title);

void dengine_window_glfw_set_monitor(GLFWmonitor* monitor, int xpos, int ypos, int refresh_rate);

void dengine_window_glfw_set_swapinterval(int interval);

GLFWwindow* dengine_window_glfw_get_currentwindow();

void dengine_window_glfw_pollevents();

void dengine_window_glfw_context_makecurrent(GLFWwindow* window);

int dengine_window_glfw_context_gladloadgl();

#endif // DENGINE_WIN_GLFW

#ifdef DENGINE_ANDROID
void dengine_window_android_set_nativewindow(struct ANativeWindow* window);

int dengine_window_android_egl_context_gladloadgl();
#endif

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // WINDOW_H



