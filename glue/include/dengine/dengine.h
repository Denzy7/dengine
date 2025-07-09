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
#include <dengine-utils/types.h>
#ifdef DENGINE_ANDROID
#include <dengine-utils/platform/android.h>
#endif

#include <dengine-gui/gui.h>

#include <dengine-model/model.h>

#include <dengine-scene/scene.h>

#include <dengine-script/script.h>
#ifdef DENGINE_SCRIPTING_PYTHON
#include <dengine-script/python/python.h>
#endif

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

    int gui_subdata;

    int cache_textures;
    int cache_shaders;

    int enable_logthread;

    int android_handlebackbutton;
    Stream assets_zip_stream;
    ZipRead assets_zip;
}DengineInitOpts;


typedef struct
{
    int isdown, snap_x, snap_y;
    int isregion_ondown;
}SWInput_Joystick;


#ifdef __cplusplus
extern "C" {
#endif

const char* dengine_get_license();
DengineInitOpts* dengine_init_get_opts();
int dengine_init();
void dengine_terminate();
int dengine_update();
/*!
 * \brief load asset to memory. mem is heap allocated, so free when done
 * \param path path to asset in android apk or file system or assetdir
 * \param mem destination of loaded asset in memory
 * \param length optional destination length of memory block 
 * \return 0 on error, else success
 */
int dengine_load_asset(const char* path, void** mem, size_t* length);

/*!
 * \brief software input joystick 
 * \param x x screen coordinate
 * \param y y screen coordinate
 * \param dim width and height of joystick. it must be a square after all
 * \param snap_lim if > 0, allows the joystick to snap if input is around x and y around the limit region specified
 * \param clamp a texture for the clamp limiting the handle. usually a hollow circle
 * \param handle a texture for the handle which user operates. usually a filled circle
 * \param outx outputs -1 and 1 with how much of the handle has deviated from centre of clamp in x axis
 * \param outy outputs -1 and 1 with how much of the handle has deviated from centre of clamp in x axis
 * \param store static private datastore which should be init to 0 with memset
 */
void dengine_input_swinput_joystick(
        int x, int y, int dim, 
        int snap_lim,
        Texture* clamp, float* clamp_color_vec4,
        Texture* handle, float* handle_color_vec4, 
        float* outx, float* outy,
        SWInput_Joystick* joystick);

#ifdef __cplusplus
}
#endif
#endif // DENGINE_H
