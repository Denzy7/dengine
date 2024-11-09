#include <dengine/dengine.h>

#include <string.h> //memset
#ifdef DENGINE_ANDROID
#define SWBTNS
#endif
#define SWBTNS
int testdengine_lighting_normalmap(int argc, char **argv)
{
    const char* GL = (const char*)glGetString(GL_VERSION);

    const size_t prtbuf_sz = 1024;
    char* prtbuf = malloc(prtbuf_sz);

    Shader stdshdr,dftshdr ;
    dengine_shader_make_standard(DENGINE_SHADER_STANDARD, &stdshdr);
    dengine_shader_make_standard(DENGINE_SHADER_DEFAULT, &dftshdr);

    Material plane_mat;
    dengine_material_setup(&plane_mat);
    dengine_material_set_shader_color(&stdshdr, &plane_mat);

    File2Mem f2m;
    Texture diffuseTex;
    memset(&diffuseTex, 0, sizeof(Texture));
    diffuseTex.interface = DENGINE_TEXTURE_INTERFACE_8_BIT;
    diffuseTex.auto_dataonload = 1;
    const char* brickfile = "textures/2d/brickwall.jpg";
#ifdef DENGINE_ANDROID
    f2m.file = brickfile;
    dengineutils_android_asset2file2mem(&f2m);
    dengine_texture_load_mem(f2m.mem, f2m.size, 1, &diffuseTex);
    free(f2m.mem);
#else
    snprintf(prtbuf, prtbuf_sz, "%s/%s", dengineutils_filesys_get_assetsdir(), brickfile);
    dengine_texture_load_file(prtbuf, 1, &diffuseTex);
#endif
    Texture normalTex;
    memset(&normalTex, 0, sizeof(Texture));
    normalTex.interface = DENGINE_TEXTURE_INTERFACE_8_BIT;
    normalTex.auto_dataonload = 1;
    const char* brickfile_n = "textures/2d/brickwall_normal.jpg";
#ifdef DENGINE_ANDROID
    f2m.file = brickfile_n;
    dengineutils_android_asset2file2mem(&f2m);
    dengine_texture_load_mem(f2m.mem, f2m.size, 1, &normalTex);
    free(f2m.mem);
#else
    snprintf(prtbuf, prtbuf_sz, "%s/%s", dengineutils_filesys_get_assetsdir(),brickfile_n);
    dengine_texture_load_file(prtbuf, 1, &normalTex);
#endif

    dengine_material_set_texture(&diffuseTex, "diffuseTex", &plane_mat);
    dengine_material_set_texture(&normalTex, "normalTex", &plane_mat);

    Primitive axis;
    dengine_primitive_gen_axis(&axis, &dftshdr);

    Primitive plane, cube;
    dengine_primitive_gen_plane(&plane, &stdshdr);
    dengine_primitive_gen_cube(&cube, &stdshdr);

    PointLight pLight;
    memset(&pLight, 0, sizeof(PointLight));
    dengine_lighting_setup_pointlight(&pLight);

    pLight.position[0] = 0.5f;
    pLight.position[1] = 2.5f;
    pLight.position[2] = 1.5f;

    pLight.light.strength = 2.0f;

    Camera camera;
    dengine_camera_setup(&camera);
    dengine_camera_apply(&stdshdr, &camera);
    dengine_camera_apply(&dftshdr, &camera);

    mat4 model;
    vec3 scale = {5.0f, 5.0f, 5.0f};
    vec3 position = {0.0f, 1.0f, 0.0f};
    vec4 yellow = {1.0f, 1.0f, 0.0f, 1.0f};

    char fps[100];
    snprintf(fps, sizeof(fps), "FPS : 0.0 (0.0ms)");
    double elapsed = 0;
    float fontsz = denginegui_get_fontsz();

    glLineWidth(3.0f);

#ifdef SWBTNS
    Texture joy, joyhand;
    float red[4] = {1.0, 0.0, 0.0, 1.0};
    float green[4] = {0.0, 1.0, 0.0, 1.0};
    Texture* texptr[2] = {&joy, &joyhand};
    static const char* joytexstrs[] =
    {
        "textures/2d/clamp.png",
        "textures/2d/handle.png",
    };
    memset(&joy, 0, sizeof(joy));
    memset(&joyhand, 0, sizeof(joyhand));
    joy.auto_dataonload = 1;
    joyhand.auto_dataonload = 1;
    joy.interface = DENGINE_TEXTURE_INTERFACE_8_BIT;
    joyhand.interface = DENGINE_TEXTURE_INTERFACE_8_BIT;
    for(int i = 0; i < 2; i++)
    {
#ifdef DENGINE_ANDROID
        File2Mem f2m;
        f2m.file = joytexstrs[i];
        dengineutils_android_asset2file2mem(&f2m);
        dengine_texture_load_mem(f2m.mem, f2m.size, 1, texptr[i]);
#else
        snprintf(prtbuf, prtbuf_sz, "%s/%s", 
                dengineutils_filesys_get_assetsdir(),
                joytexstrs[i]);
        dengine_texture_load_file(prtbuf, 1, texptr[i]);
#endif
    }
#endif

    /*TODO: before we actually get a working lighting system,
     * we set number of lights manually */
    dengine_shader_set_int(&stdshdr, "nr_pointLights", 1);
    
    while(dengine_update())
    {
        dengine_material_use(&plane_mat);

        glm_mat4_identity(model);
        glm_scale(model, scale);
        dengine_shader_set_mat4(&stdshdr, "model", model[0]);
        dengine_draw_primitive(&plane, &stdshdr);

        glm_mat4_identity(model);
        glm_translate(model, position);
        dengine_shader_set_mat4(&stdshdr, "model", model[0]);
        dengine_draw_primitive(&cube, &stdshdr);

        double delta = dengineutils_timer_get_delta() / 1000.0;
        elapsed += delta;
        if(elapsed > 1.0)
        {
            snprintf(fps, sizeof(fps), "FPS : %.1f (%.1fms)", 1 / delta, delta * 1000);
            elapsed = 0;
        }
        static const float speed = 6.0f;

        dengine_lighting_apply_pointlight(&pLight, &stdshdr);

        if(dengine_input_get_key('W') || dengine_input_gamepad_get_btn(DENGINE_INPUT_PAD0, DENGINE_INPUT_PAD_BUTTON_DPAD_UP))
            pLight.position[2] -= delta * speed;

        if(dengine_input_get_key('S') || dengine_input_gamepad_get_btn(DENGINE_INPUT_PAD0, DENGINE_INPUT_PAD_BUTTON_DPAD_DOWN))
            pLight.position[2] += delta * speed;

        if(dengine_input_get_key('A') || dengine_input_gamepad_get_btn(DENGINE_INPUT_PAD0, DENGINE_INPUT_PAD_BUTTON_DPAD_LEFT))
            pLight.position[0] -= delta * speed;

        if(dengine_input_get_key('D') || dengine_input_gamepad_get_btn(DENGINE_INPUT_PAD0, DENGINE_INPUT_PAD_BUTTON_DPAD_RIGHT))
            pLight.position[0] += delta * speed;

        if(dengine_input_get_key('E') || dengine_input_gamepad_get_btn(DENGINE_INPUT_PAD0, DENGINE_INPUT_PAD_BUTTON_A))
            pLight.position[1] += delta * speed;

        if(dengine_input_get_key('C') || dengine_input_gamepad_get_btn(DENGINE_INPUT_PAD0, DENGINE_INPUT_PAD_BUTTON_X))
            pLight.position[1] -= delta * speed;

        glm_mat4_identity(model);
        glm_translate(model, pLight.position);
        dengine_shader_set_mat4(&dftshdr, "model", model[0]);
        float axiscol[3];
        for (int i = 0; i < 3; i++) {
            memset(axiscol, 0, sizeof(axiscol));
            axiscol[i] = 1.0f;
            axis.index_count = 2;
            axis.offset = (void*)(i*2*sizeof (uint16_t));
            dengine_shader_set_vec3(&dftshdr, "color", axiscol);
            dengine_draw_primitive(&axis, &dftshdr);
        }
        
#ifdef SWBTNS
        int w, h;

        dengine_viewport_get(NULL, NULL, &w, &h);
        float joydim = 300.0f;
        float joyoff = 100.0f;
        float joyhanddim = joydim / 2.0f;
        float msex, msey;
        static float joyx, joyy; 
        static float joyhandx, joyhandy;
        static int joydown = 0;
        vec2 input = {0.0f, 0.0f};

        msex = dengine_input_get_mousepos_x();
        msey= dengine_input_get_mousepos_y();

        if(dengine_input_get_mousebtn(DENGINE_INPUT_MSEBTN_PRIMARY))
        {
            if(!joydown)
            {
                joyx = msex - (joydim / 2.0f);
                joyy = msey - (joydim / 2.0f);
                joydown = 1;
            }

            /* extract original mouse pos */
            vec2 a = {joyx + (joydim / 2.0f), joyy + (joydim / 2.0f)};
            vec2 b = {msex, msey};
            glm_vec2_sub(b, a, input);
            glm_vec2_divs(input, (joydim / 2.0f), input);

            joyhandx = msex - (joyhanddim / 2.0f);
            joyhandy = msey - (joyhanddim / 2.0f);
                
        }else {
            joyx = w  - joyoff - joydim;
            joyy = joyoff;
            joydown = 0;
            joyhandx = joyx + (joydim / 2.0f) - (joyhanddim / 2.0f);
            joyhandy = joyy + (joydim / 2.0f) - (joyhanddim / 2.0f);
        }
        denginegui_set_panel_discard(1);
        denginegui_panel(joyx, joyy, joydim, joydim, &joy, NULL, red);
        denginegui_panel(joyhandx, joyhandy, joyhanddim, joyhanddim, &joyhand, NULL, green);
        denginegui_set_panel_discard(0);
        
        denginegui_set_button_repeatable(1);
        if(denginegui_button(joyoff, joyoff + joydim / 2.0f, joydim / 2.0f, joydim / 2.0f, "E", NULL))
            pLight.position[1] += delta * speed;
        if(denginegui_button(joyoff, joyoff, joydim / 2.0f, joydim / 2.0f, "C", NULL))
            pLight.position[1] -= delta * speed;
        
        denginegui_set_button_repeatable(0);
        pLight.position[2] -= (delta * speed * input[1]);
        pLight.position[0] += (delta * speed * input[0]);
#endif

        denginegui_text(10, 10, GL, NULL);

        denginegui_text(10, 10 + fontsz, fps, yellow);

        denginegui_text(10, 10 + 3 * fontsz, "WASD(DPAD,JOYSTICK) = MOVE LIGHT, EC(A,X) = UP/DOWN", NULL);
    }
    free(prtbuf);
    dengine_material_destroy(&plane_mat);

    return 0;
}

#ifndef DENGINE_ANDROID
int main(int argc, char** argv)
{
    DengineInitOpts* opts = dengine_init_get_opts();
    opts->window_title = "testdengine-lighting-normalmap";
    dengine_init();
    testdengine_lighting_normalmap(argc, argv);
    dengine_terminate();
}
#endif
