#include "dengine/dengine.h"

#include <string.h>

DengineInitOpts opts;
int hassetopts = 0;

int dengine_init()
{
    dengine_init_set_opts(dengine_init_get_defopts());

    //All this to GL initialization
    if(!dengine_window_init())
        return 0;

    Window window;
    if(!dengine_window_create(opts.window_width, opts.window_height, opts.window_title, &window))
        return 0;

    dengine_window_makecurrent(&window);

    if(!dengine_window_loadgl())
        return 0;

    //INPUT
    dengine_input_init();

    //DEBUGGING, INCASE OF SIGSEGV OR SIGABRT
    dengineutils_debug_init();

    //GUI. SET FONT TOO
    if(!denginegui_init())
        return 0;

    denginegui_set_font(NULL, opts.font_size, opts.font_bitmapsize);

    return 1;
}

void dengine_update()
{
    dengine_window_swapbuffers();
    dengine_input_pollevents();
}

void dengine_terminate()
{
    dengineutils_debug_terminate();

    dengine_window_terminate();
}

void dengine_init_set_opts(DengineInitOpts* _opts)
{
    hassetopts = 1;
    opts = *_opts;
}

DengineInitOpts* dengine_init_get_defopts()
{
    memset(&opts, 0, sizeof(DengineInitOpts));

    opts.window_height = 720;
    opts.window_width = 1280;
    opts.window_title = "Dengine!";

    opts.font_size = 18.0f;
    opts.font_bitmapsize = 512;

    return &opts;
}
