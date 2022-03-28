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
    int window_msaa;

    int gl_max;
    int gl_min;
    int gl_core;

    int enable_depth;
    int enable_backfaceculling;

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

#include <dengine-utils/logging.h>
#include <dengine-utils/filesys.h>
#include <dengine-utils/debug.h>
#include <dengine-utils/os.h>
#include <dengine-utils/rng.h>
#include <dengine-utils/timer.h>

#include <dengine-gui/gui.h>

#include <dengine-model/model.h>

#include <dengine-scene/scene.h>

#ifdef __cplusplus
extern "C" {
#endif

int dengine_init();

void dengine_terminate();

void dengine_update();

DengineInitOpts* dengine_init_get_opts();

#ifdef __cplusplus
}
#endif

#endif // DENGINE_H
