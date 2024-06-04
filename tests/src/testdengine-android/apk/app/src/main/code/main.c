#include <dengine/dengine.h>

typedef struct 
{
    int(*mainfun)(int,char**);
    void(*setwindowfun)(DengineWindow*);
    int argc;
    char**argv;
}DengineAndroidTest;


char* testdengine_lighting_normalmap_argv[] = {
    "lighting_normalmap"
};
int testdengine_lighting_normalmap(int argc, char **argv);

char* testdengine_script_nslbullet_car_argv[] = 
{
    "nslbullet_car"
};
int testdengine_script_nslbullet_car_main(int argc, char **argv);

char* testdengine_lighting_standard_argv[] =
{
    "lighting_standard"
};
int testdengine_lighting_standard(int argc, char** argv);
char* testdengine_gui_textbench_argv[] =
{
    "gui_textbench"
};
int testdengine_gui_textbench(int argc, char** argv);

void testdengine_lighting_standard_setwindow(DengineWindow*);

DengineAndroidTest tests[] = 
{
    {
        testdengine_lighting_normalmap,
        NULL,
        DENGINE_ARY_SZ(testdengine_lighting_normalmap_argv),
        testdengine_lighting_normalmap_argv
    },
    {
        testdengine_script_nslbullet_car_main,
        NULL,
        DENGINE_ARY_SZ(testdengine_lighting_normalmap_argv),
        testdengine_script_nslbullet_car_argv
    },
    {
        testdengine_lighting_standard,
        testdengine_lighting_standard_setwindow,
        DENGINE_ARY_SZ(testdengine_lighting_standard_argv),
        testdengine_lighting_standard_argv
    },
    {
        testdengine_gui_textbench,
        NULL,
        DENGINE_ARY_SZ(testdengine_gui_textbench_argv),
        testdengine_gui_textbench_argv
    },
};

void android_main(struct android_app* app)
{

    int w, h;
    int status;
    size_t i;
    float fontsz;
    float btnwid;
    float btnhgt;
    float pad = 5.0f;
    DengineInitOpts* initopts;

    dengineutils_logging_log("DENGINE_TESTS: begin!");

    dengineutils_android_set_app(app);
    initopts = dengine_init_get_opts();
    initopts->android_handlebackbutton = 1;
    dengine_init();

    fontsz = denginegui_get_fontsz();
    dengine_viewport_get(NULL, NULL, &w, &h);    

    /*btnhgt = ((float)h - fontsz - pad) / (float)DENGINE_ARY_SZ(tests);*/
    btnhgt = 100.0f;
    btnwid = w - fontsz;

    while(1)
    {
        if(app->destroyRequested)
        {
            dengineutils_logging_log("destroyRequested=1, break while(1)");
            break;
        }

        /* since we handle back btn, we'll have to finish
         * manually
         */
        if(!dengine_update())
            ANativeActivity_finish(app->activity);

        /* should probably use grid by finding 2 factors for all 
         * programs then dividing 
         * the elements then build grid, but one row will work for
         * now!
         */
        denginegui_text(fontsz, h - fontsz, "DENGINE TESTS. CLICK ANY TEST BELOW (press back to return here for tests that support it)", NULL);
        for(i = 0; i < DENGINE_ARY_SZ(tests); i++)
        {
           if(denginegui_button(fontsz,
                       h - fontsz - pad - btnhgt - (i * btnhgt) - (i * pad), 
                       btnwid, btnhgt, tests[i].argv[0], NULL))
           {
               if(tests[i].setwindowfun)
                   tests[i].setwindowfun(initopts->window);
               status = tests[i].mainfun(tests[i].argc, tests[i].argv);
               break;
           }
        }
    }
    dengine_terminate();
    dengineutils_logging_log("DENGINE_TESTS: exited [%d]", status);
    return;
}
