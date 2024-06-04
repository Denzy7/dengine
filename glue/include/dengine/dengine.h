#ifndef DENGINE_H
#define DENGINE_H

#include <string.h> //memset

/*
 * Includes all the stuff we'd expect to use
 */

#include <dengine/window.h>
#include <dengine/input.h>
#include <dengine/primitive.h>
#include <dengine/material.h>
#include <dengine/lighting.h>
#include <dengine/draw.h>
#include <dengine/loadgl.h>
#include <dengine/viewport.h>
#include <dengine/entrygl.h>

#include <dengine-utils/macros.h>
#include <dengine-utils/logging.h>
#include <dengine-utils/filesys.h>
#include <dengine-utils/debug.h>
#include <dengine-utils/os.h>
#include <dengine-utils/rng.h>
#include <dengine-utils/timer.h>
#include <dengine-utils/confserialize.h>
#include <dengine-utils/zipread.h>
#ifdef DENGINE_ANDROID
#include <dengine-utils/platform/android.h>
#endif

#include <dengine-gui/gui.h>

#include <dengine-model/model.h>

#include <dengine-scene/scene.h>

#include <dengine-script/script.h>

typedef struct
{
    DengineWindow* window;
    int window_width;
    int window_height;
    const char* window_title;
    int window_msaa;
    int window_createnative;
    int window_swapinterval;

    int gl_max;
    int gl_min;
    int gl_core;
    int gl_loaddefault;

    int enable_depth;
    int enable_backfaceculling;

    float font_size;
    int font_bitmapsize;

    int cache_textures;
    int cache_shaders;

    int enable_logthread;

    int android_handlebackbutton;
}DengineInitOpts;
#ifdef __cplusplus
extern "C" {
#endif

const char* dengine_get_license();
DengineInitOpts* dengine_init_get_opts();
int dengine_init();
void dengine_terminate();
int dengine_update();
/*!
 * \brief load asset to memory
 * \param path path to asset in android apk or file system or assetdir
 * \param mem destination of loaded asset in memory
 * \param length optional destination length of memory block 
 * \return 0 on error, else success
 */
int dengine_load_asset(const char* path, void** mem, size_t* length);

#ifdef __cplusplus
}
#endif
#endif // DENGINE_H
