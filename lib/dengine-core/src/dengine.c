#include "dengine/dengine.h"

#include <string.h>

DengineInitOpts opts;
int hasgotopts = 0;

int dengine_init()
{
    if(!hasgotopts)
        opts = *dengine_init_get_opts();

    //All this to GL initialization
    if(!dengine_window_init())
        return 0;

#ifndef DENGINE_ANDROID
    //Android creates a window already in init... how convinient and limiting?
    Window window;
    if(!dengine_window_create(opts.window_width, opts.window_height, opts.window_title, &window))
        return 0;

     dengine_window_makecurrent(&window);
#endif

    if(!dengine_window_loadgl())
        return 0;

    //INPUT
    dengine_input_init();

    //DEBUGGING, INCASE OF SIGSEGV OR SIGABRT
    dengineutils_debug_init();

    //ALLOCATE FILESYS DIRECTORIES
    dengineutils_filesys_init();

    //GUI. SET FONT TOO
    if(!denginegui_init())
        return 0;

    denginegui_set_font(NULL, opts.font_size, opts.font_bitmapsize);

    //SEED RNG. NOT MT-SAFE!(AFAIK)
    dengineutils_rng_set_seedwithtime();

    //depth testing ✅
    if(opts.enable_depth)
        glEnable(GL_DEPTH_TEST);

    //backface culling. save draw calls ✅
    if(opts.enable_backfaceculling)
        glEnable(GL_CULL_FACE);

    return 1;
}

void dengine_update()
{
    dengineutils_timer_update();
    dengine_window_swapbuffers();
    dengine_input_pollevents();
}

void dengine_terminate()
{
    denginegui_terminate();
    dengineutils_filesys_terminate();

    dengineutils_debug_terminate();
    dengine_window_terminate();
}

DengineInitOpts* dengine_init_get_opts()
{
    memset(&opts, 0, sizeof(DengineInitOpts));

    opts.window_height = 720;
    opts.window_width = 1280;
    opts.window_title = "Dengine!";

    opts.font_size = 18.0f;
    opts.font_bitmapsize = 512;

    opts.enable_backfaceculling = 1;
    opts.enable_depth = 1;

    hasgotopts = 1;

    return &opts;
}
