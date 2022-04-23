#ifndef DENGINE_H
#define DENGINE_H

#include <string.h> //memset

/*
 * Includes all the stuff we'd expect to use
 */

typedef struct
{
    int window_width;
    int window_height;
    const char* window_title;
    int window_msaa;
    int window_createnative;

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
}DengineInitOpts;

#include <dengine/window.h>
#include <dengine/input.h>
#include <dengine/primitive.h>
#include <dengine/material.h>
#include <dengine/lighting.h>
#include <dengine/draw.h>
#include <dengine/loadgl.h>
#include <dengine/macros.h>
#include <dengine/viewport.h>
#include <dengine/entrygl.h>
#ifdef DENGINE_ANDROID
#include <dengine/android.h>
#endif

#include <dengine-utils/logging.h>
#include <dengine-utils/filesys.h>
#include <dengine-utils/debug.h>
#include <dengine-utils/os.h>
#include <dengine-utils/rng.h>
#include <dengine-utils/timer.h>
#include <dengine-utils/confserialize.h>

#include <dengine-gui/gui.h>

#include <dengine-model/model.h>

#include <dengine-scene/scene.h>

static DengineInitOpts DENGINE_INIT_OPTS;
static int DENGINE_HAS_GOT_INIT_OPTS = 0;

DENGINE_INLINE DengineInitOpts* dengine_init_get_opts()
{
    memset(&DENGINE_INIT_OPTS, 0, sizeof(DENGINE_HAS_GOT_INIT_OPTS));

    //ALLOCATE FILESYS DIRECTORIES
    dengineutils_filesys_init();

    DENGINE_INIT_OPTS.window_height = 720;
    DENGINE_INIT_OPTS.window_width = 1280;
    DENGINE_INIT_OPTS.window_title = "Dengine!";
    DENGINE_INIT_OPTS.window_msaa = 4;
    DENGINE_INIT_OPTS.window_createnative = 1;

    DENGINE_INIT_OPTS.font_size = 18.0f;
    DENGINE_INIT_OPTS.font_bitmapsize = 512;

    DENGINE_INIT_OPTS.enable_backfaceculling = 1;
    DENGINE_INIT_OPTS.enable_depth = 1;

    DENGINE_INIT_OPTS.cache_shaders = 1;

    const size_t prtbf_sz = 2048;
    char* prtbf = (char*) malloc(prtbf_sz);

    snprintf(prtbf, prtbf_sz, "%s/dengine", dengineutils_filesys_get_filesdir());

    if(!dengineutils_os_direxist(prtbf))
        dengineutils_os_mkdir(prtbf);

    snprintf(prtbf, prtbf_sz, "%s/dengine/dengine.ini", dengineutils_filesys_get_filesdir());

    Conf* conf = dengineutils_confserialize_new(prtbf, '=');
    if(fopen(prtbf, "r"))
    {
        dengineutils_confserialize_load(conf, 1);

        // WINDOW
        char* window_width = dengineutils_confserialize_get("window_width", conf);
        if(window_width)
            sscanf(window_width, "%d", &DENGINE_INIT_OPTS.window_width);

        char* window_height = dengineutils_confserialize_get("window_height", conf);
        if(window_height)
            sscanf(window_height, "%d", &DENGINE_INIT_OPTS.window_height);

        char* window_msaa = dengineutils_confserialize_get("window_msaa", conf);
        if(window_msaa)
            sscanf(window_msaa, "%d", &DENGINE_INIT_OPTS.window_msaa);

        // FONT
        char* font_size = dengineutils_confserialize_get("font_size", conf);
        if(font_size)
            sscanf(font_size, "%f", &DENGINE_INIT_OPTS.font_size);

        char* font_bitmapsize = dengineutils_confserialize_get("font_bitmapsize", conf);
        if(font_bitmapsize)
            sscanf(font_bitmapsize, "%d", &DENGINE_INIT_OPTS.font_bitmapsize);

        //CACHING
        char* cache_textures = dengineutils_confserialize_get("cache_textures", conf);
        if(cache_textures)
            sscanf(cache_textures, "%d", &DENGINE_INIT_OPTS.cache_textures);

        char* cache_shaders = dengineutils_confserialize_get("cache_shaders", conf);
        if(cache_shaders)
            sscanf(cache_shaders, "%d", &DENGINE_INIT_OPTS.cache_shaders);


    }else
    {
        dengineutils_confserialize_put_block("window", conf);
        snprintf(prtbf, prtbf_sz, "%d", DENGINE_INIT_OPTS.window_width);
        dengineutils_confserialize_put("window_width", prtbf, conf);

        snprintf(prtbf, prtbf_sz, "%d", DENGINE_INIT_OPTS.window_height);
        dengineutils_confserialize_put("window_height", prtbf, conf);

        snprintf(prtbf, prtbf_sz, "%d", DENGINE_INIT_OPTS.window_msaa);
        dengineutils_confserialize_put_comment("would you like screen multisampled anti aliasing?", conf);
        dengineutils_confserialize_put("window_msaa", prtbf, conf);

        dengineutils_confserialize_put_newline(conf);

        dengineutils_confserialize_put_block("fontconfigs", conf);
        snprintf(prtbf, prtbf_sz, "%.1f", DENGINE_INIT_OPTS.font_size);
        dengineutils_confserialize_put_comment("fontsize in pixel size", conf);
        dengineutils_confserialize_put("font_size", prtbf, conf);

        snprintf(prtbf, prtbf_sz, "%d", DENGINE_INIT_OPTS.font_bitmapsize);
        dengineutils_confserialize_put_comment("size of the initial font bitmap", conf);
        dengineutils_confserialize_put("font_bitmapsize", prtbf, conf);

        dengineutils_confserialize_put_newline(conf);

        dengineutils_confserialize_put_block("caching", conf);
        snprintf(prtbf, prtbf_sz, "%d", DENGINE_INIT_OPTS.cache_textures);
        dengineutils_confserialize_put_comment("cache textures to disk. may improve loading speeds "
                                               "at the cost of higher disk usage", conf);
        dengineutils_confserialize_put("cache_textures", prtbf, conf);

        snprintf(prtbf, prtbf_sz, "%d", DENGINE_INIT_OPTS.cache_shaders);
        dengineutils_confserialize_put_comment("cache shaders to disk. always use this option "
                                               "if your hardware supports it", conf);
        dengineutils_confserialize_put("cache_shaders", prtbf, conf);

        dengineutils_confserialize_put_newline(conf);

        dengineutils_confserialize_write(conf);
    }
    dengineutils_confserialize_free(conf);

    free(prtbf);

    DENGINE_HAS_GOT_INIT_OPTS = 1;

    return &DENGINE_INIT_OPTS;
}


DENGINE_INLINE int dengine_init()
{
    if(!DENGINE_HAS_GOT_INIT_OPTS)
        DENGINE_INIT_OPTS = *dengine_init_get_opts();

    if(DENGINE_INIT_OPTS.window_msaa)
        dengine_window_request_MSAA(DENGINE_INIT_OPTS.window_msaa);

    if(DENGINE_INIT_OPTS.gl_max)
        dengine_window_request_GL(DENGINE_INIT_OPTS.gl_max,
                                  DENGINE_INIT_OPTS.gl_min,
                                  DENGINE_INIT_OPTS.gl_core);

    //All this to GL initialization
    if(DENGINE_INIT_OPTS.window_createnative)
    {
        if(!dengine_window_init())
            return 0;

#ifndef DENGINE_ANDROID
        //Android creates a window already in init... how convinient and limiting?
        Window window;
        if(!dengine_window_create(DENGINE_INIT_OPTS.window_width, DENGINE_INIT_OPTS.window_height, DENGINE_INIT_OPTS.window_title, &window))
            return 0;

         dengine_window_makecurrent(&window);
#endif
    }

    if(DENGINE_INIT_OPTS.gl_loaddefault)
    {
        //use default glad loader
        if(!gladLoadGL())
            return 0;
    }else
    {
        //use window loader
        if(!dengine_window_loadgl())
            return 0;
    }

    //caching
    dengine_texture_set_texturecache(DENGINE_INIT_OPTS.cache_textures);
    dengine_shader_set_shadercache(DENGINE_INIT_OPTS.cache_shaders);

    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    int samples;
    glGetIntegerv(GL_SAMPLES, &samples);
    const char* GL = (const char*)glGetString(GL_VERSION);
    const char* GLSL = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
    const char* VENDOR = (const char*)glGetString(GL_VENDOR);
    const char* RENDERDER = (const char*)glGetString(GL_RENDERER);
    char msaastr[9];
    if(samples)
        snprintf(msaastr, 9, "%dx MSAA", samples);
    else
        snprintf(msaastr, 9, "NO MSAA");

    dengineutils_logging_log("TODO::DENGINE : %s", DENGINE_VERSION);
    dengineutils_logging_log("INFO::GL : %s\nGLSL : %s\nVENDOR : %s\nRENDERDER : %s\n"
                             "VIEWPORT : %dx%d %s",
                             GL, GLSL, VENDOR, RENDERDER,
                             viewport[2], viewport[3], msaastr);
    //INPUT
    dengine_input_init();

    //DEBUGGING, INCASE OF SIGSEGV OR SIGABRT
    dengineutils_debug_init();

#ifdef DENGINE_ANDROID
    //Can safely set files and cachedirs
    dengine_android_set_filesdir();
    dengine_android_set_cachedir();
#endif

    //GUI. SET FONT TOO
    if(!denginegui_init())
        return 0;

    denginegui_set_font(NULL, DENGINE_INIT_OPTS.font_size, DENGINE_INIT_OPTS.font_bitmapsize);

    //SEED RNG. NOT MT-SAFE!(AFAIK)
    dengineutils_rng_set_seedwithtime();

    //depth testing ✅
    if(DENGINE_INIT_OPTS.enable_depth)
        glEnable(GL_DEPTH_TEST);

    //backface culling. save draw calls ✅
    if(DENGINE_INIT_OPTS.enable_backfaceculling)
        glEnable(GL_CULL_FACE);

    return 1;
}

DENGINE_INLINE void dengine_terminate()
{
    denginegui_terminate();
    dengineutils_filesys_terminate();

    dengineutils_debug_terminate();
    dengine_window_terminate();
}

DENGINE_INLINE void dengine_update()
{
    dengineutils_timer_update();
    dengine_window_swapbuffers();
    dengine_input_pollevents();
}



#endif // DENGINE_H
