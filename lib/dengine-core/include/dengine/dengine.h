#ifndef DENGINE_H
#define DENGINE_H

/*
 * Includes all the stuff we'd expect to use
 */

typedef struct
{
    int window_width;
    int window_height;
    char* window_title;

    int gl_max;
    int gl_min;
    int gl_core;

    float font_size;
    int font_bitmapsize;
}DengineInitOpts;

#include <dengine/window.h>
#include <dengine/input.h>
#include <dengine/primitive.h>
#include <dengine/material.h>
#include <dengine/lighting.h>
#include <dengine/draw.h>
#include <dengine/loadgl.h>
#include <dengine/macros.h>
#ifdef DENGINE_ANDROID
#include <dengine/android.h>
#endif

#include <dengine-utils/filesys.h>
#include <dengine-utils/debug.h>

#include <dengine-gui/gui.h>

#include <dengine-model/model.h>

#include <dengine-scene/scene.h>

#ifdef __cplusplus
extern "C" {
#endif

int dengine_init();

void dengine_terminate();

void dengine_update();

void dengine_init_set_opts(DengineInitOpts* _opts);

DengineInitOpts* dengine_init_get_defopts();

#ifdef __cplusplus
}
#endif

#endif // DENGINE_H
