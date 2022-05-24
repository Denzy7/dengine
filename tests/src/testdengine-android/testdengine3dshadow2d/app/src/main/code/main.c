#include <dengine/dengine.h>

Primitive cube, plane;
Material material;
Camera camera;
DirLight dLight;
Shader shadow2d, std;
mat4 model;
vec3 pos = {0.0f, 1.0f, 0.0f};
vec3 scale = {5.0f, 5.0f, 5.0f};

static void init(struct android_app* app)
{
    if(dengine_init())
    {
        dengine_shader_make_standard(DENGINE_SHADER_STANDARD, &std);
        dengine_shader_make_standard(DENGINE_SHADER_SHADOW2D, &shadow2d);

        dengine_primitive_gen_cube(&cube, &std);
        dengine_primitive_gen_plane(&plane, &std);

        memset(&dLight, 0, sizeof (DirLight));
        dLight.shadow.enable = 1;
        dLight.shadow.shadow_map_size = 512;
        dengine_lighting_setup_dirlight(&dLight);

        dengine_camera_setup(&camera);
        dengine_camera_apply(&std, &camera);

        dengine_material_setup(&material);
        dengine_material_set_shader_color(&std, &material);

        dengine_material_set_texture(&dLight.shadow.shadow_map.depth, "dLightShadow", &material);
    }
}

static void term(struct  android_app* app)
{
    dengine_material_destroy(&material);
    dengine_terminate();
}

static void draw()
{
    dengine_update();

    double delta = dengineutils_timer_get_delta() / 1000.0;

    dengine_lighting_apply_dirlight(&dLight, &std);

    //SHADOW PASS
    dengine_lighting_shadowop_clear(&dLight.shadow);

    glm_mat4_identity(model);
    glm_scale(model, scale);
    dengine_lighting_shadow_dirlight_draw(&dLight, &shadow2d, &plane, model[0]);

    glm_mat4_identity(model);
    glm_translate(model, pos);
    dengine_lighting_shadow_dirlight_draw(&dLight, &shadow2d, &cube, model[0]);

    //COLOR PASS
    dengine_material_use(&material);

    glm_mat4_identity(model);
    glm_scale(model, scale);
    dengine_shader_set_mat4(&std, "model", model[0]);
    dengine_draw_primitive(&plane, &std);

    glm_mat4_identity(model);
    glm_translate(model, pos);
    dengine_shader_set_mat4(&std, "model", model[0]);
    dengine_draw_primitive(&cube, &std);

    dengine_material_use(NULL);

    denginegui_panel(0, 0, 256, 256, &dLight.shadow.shadow_map.depth, NULL, NULL);

    if(dengine_input_get_key('E'))
        pos[1] += delta * 5.0f;

    if(dengine_input_get_key('C'))
        pos[1] -= delta * 5.0f;
}

void android_main(struct android_app* state)
{
    dengineutils_android_set_app(state);
    dengineutils_android_set_initfunc(init);
    dengineutils_android_set_terminatefunc(term);

    if(state->savedState)
    {
        dengineutils_logging_log("Restoring from save state");
    }

    while(1)
    {
        dengine_window_poll(DENGINE_WINDOW_CURRENT);
        //Quit and detach
        if(state->destroyRequested != 0)
        {
            dengineutils_logging_log("Destroy Requested");
            dengineutils_logging_log("Goodbye!");
            return;
        }

        if (dengine_window_isrunning(DENGINE_WINDOW_CURRENT))
            draw();

    }
}
