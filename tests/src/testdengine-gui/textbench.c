#include <dengine/dengine.h>

int testdengine_gui_textbench(int argc, char** argv)
{
        
    char textbuf[128];
    for(int i = 0; i < DENGINE_ARY_SZ(textbuf); i++)
    {
        textbuf[i] = dengineutils_rng_int(91) + 32;
    }

    denginegui_set_button_repeatable(0);
    while(dengine_update()){
        float fontsz = denginegui_get_fontsz();
        int vw, vh;
        char fpsstr[64];
        static double elapsed = 0, delta;
        static int frames = 0;
        float yellow[]= {1.0f, 1.0f, 0.0f, 1.0f};
        static int mul = 1;

        delta = dengineutils_timer_get_delta();
        elapsed += delta;
        frames++;
        if(elapsed >= 1000.0)
        {
            snprintf(fpsstr, sizeof(fpsstr), "Text Benchmark: FPS:%d(%.1fms)", 
                    frames, delta);
            elapsed = 0;
            frames = 0;
        }

        dengine_viewport_get(NULL, NULL, &vw, &vh);

        denginegui_text(fontsz, vh - fontsz, fpsstr, yellow);
        int seg = vh / fontsz;

        for(int i = 1; i < seg - (seg - mul - 3); i++)
        {
            denginegui_text(fontsz, vh - fontsz - i * fontsz, textbuf, NULL);
        }
        float btnareasq = 100.0f;
        float btnoffset = 5.0f;
        if(denginegui_button(vw - btnareasq - btnoffset, vh - btnareasq - btnoffset, btnareasq, btnareasq, "+", GLM_VEC4_BLACK))
        {
            mul++;
        }
        if(denginegui_button(vw - (2.0f * btnareasq) - (2.0f * btnoffset), vh - btnareasq - btnoffset, btnareasq, btnareasq, "-", GLM_VEC4_BLACK) )
        {
            mul--;
        }
    }

    return 0;
}

#ifndef DENGINE_ANDROID
int main(int argc, char* argv[])
{
    if(!dengine_init())
        return 1;

    int ret = testdengine_gui_textbench(argc, argv);

    dengine_terminate();
    return ret;
}
#endif
