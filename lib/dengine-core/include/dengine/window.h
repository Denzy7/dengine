/*!
 * \file window.h
 * Execute window operations
 */

#ifndef WINDOW_H
#define WINDOW_H

#include "dengine_config.h" //DENGINE_WIN_, DENGINE_GL_
#include "dengine/input.h"

typedef enum
{
    DENGINE_CONTEXT_TYPE_GLX = 1,
    DENGINE_CONTEXT_TYPE_WGL,
    DENGINE_CONTEXT_TYPE_EGL
}ContextType;

#define DENGINE_WINDOW_CURRENT dengine_window_get_current()


typedef struct DengineWindow DengineWindow;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int dengine_window_init();

void dengine_window_terminate();

void dengine_window_request_GL(int gl_major, int gl_minor, int gl_core);

void dengine_window_request_MSAA(int samples);

void dengine_window_request_defaultall();

void dengine_window_request_context(ContextType type);

DengineWindow* dengine_window_create(int width, int height, const char* title, const DengineWindow* share);

void dengine_window_destroy(DengineWindow* window);

void dengine_window_get_dim(DengineWindow* window, int* width, int* height);

int dengine_window_poll(DengineWindow* window);

void dengine_window_swapbuffers(DengineWindow* window);

int dengine_window_isrunning(DengineWindow* window);

int dengine_window_loadgl(DengineWindow* window);

int dengine_window_makecurrent(DengineWindow* window);

DengineWindow* dengine_window_get_current();

int dengine_window_set_swapinterval(DengineWindow* window, int interval);

void* dengine_window_get_proc(const char* name);

int dengine_window_resize(DengineWindow* window, int width, int height);

void dengine_window_set_fullscreen(DengineWindow* window, int state);

int dengine_window_set_position(DengineWindow* window, int x, int y);

StandardInput* dengine_window_get_input(DengineWindow* window);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // WINDOW_H



