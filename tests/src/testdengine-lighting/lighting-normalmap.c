#include <dengine/dengine.h>

#include <string.h> //memset
int main(int argc, char *argv[])
{
    DengineInitOpts* opts = dengine_init_get_opts();
    opts->window_title = "testdengine-lighting-normalmap";
    dengine_init();

    const char* GL = (const char*)glGetString(GL_VERSION);

    const size_t prtbuf_sz = 1024;
    char* prtbuf = malloc(prtbuf_sz);

    Shader* stdshdr = dengine_shader_new_shader_standard(DENGINE_SHADER_STANDARD);
    Shader* dftshdr = dengine_shader_new_shader_standard(DENGINE_SHADER_DEFAULT);

    Material plane_mat;
    dengine_material_setup(&plane_mat);
    dengine_material_set_shader_color(stdshdr, &plane_mat);

    Texture diffuseTex;
    memset(&diffuseTex, 0, sizeof(Texture));
    diffuseTex.interface = DENGINE_TEXTURE_INTERFACE_8_BIT;
    diffuseTex.auto_dataonload = 1;
    snprintf(prtbuf, prtbuf_sz, "%s/textures/2d/brickwall.jpg", dengineutils_filesys_get_assetsdir());
    dengine_texture_load_file(prtbuf, 1, &diffuseTex);

    Texture normalTex;
    memset(&normalTex, 0, sizeof(Texture));
    normalTex.interface = DENGINE_TEXTURE_INTERFACE_8_BIT;
    normalTex.auto_dataonload = 1;
    snprintf(prtbuf, prtbuf_sz, "%s/textures/2d/brickwall_normal.jpg", dengineutils_filesys_get_assetsdir());
    dengine_texture_load_file(prtbuf, 1, &normalTex);

    dengine_material_set_texture(&diffuseTex, "diffuseTex", &plane_mat);
    dengine_material_set_texture(&normalTex, "normalTex", &plane_mat);

    Primitive axis;
    dengine_primitive_gen_axis(&axis, dftshdr);

    Primitive plane, cube;
    dengine_primitive_gen_plane(&plane, stdshdr);
    dengine_primitive_gen_cube(&cube, stdshdr);

    PointLight pLight;
    memset(&pLight, 0, sizeof(PointLight));
    dengine_lighting_setup_pointlight(&pLight);

    pLight.position[0] = 0.5f;
    pLight.position[1] = 2.5f;
    pLight.position[2] = 1.5f;

    pLight.light.strength = 2.0f;

    Camera camera;
    dengine_camera_setup(&camera);
    dengine_camera_apply(stdshdr, &camera);
    dengine_camera_apply(dftshdr, &camera);

    mat4 model;
    vec3 scale = {5.0f, 5.0f, 5.0f};
    vec3 position = {0.0f, 1.0f, 0.0f};
    vec4 yellow = {1.0f, 1.0f, 0.0f, 1.0f};

    char fps[100];
    snprintf(fps, sizeof(fps), "FPS : 0.0 (0.0ms)");
    double elapsed = 0;
    float fontsz = denginegui_get_fontsz();

    while(dengine_window_isrunning())
    {
        dengine_material_use(&plane_mat);

        glm_mat4_identity(model);
        glm_scale(model, scale);
        dengine_shader_set_mat4(stdshdr, "model", model[0]);
        dengine_draw_primitive(&plane, stdshdr);

        glm_mat4_identity(model);
        glm_translate(model, position);
        dengine_shader_set_mat4(stdshdr, "model", model[0]);
        dengine_draw_primitive(&cube, stdshdr);

        dengine_material_use(NULL);

        double delta = dengineutils_timer_get_delta() / 1000.0;
        elapsed += delta;
        if(elapsed > 1.0)
        {
            snprintf(fps, sizeof(fps), "FPS : %.1f (%.1fms)", 1 / delta, delta * 1000);
            elapsed = 0;
        }
        static const float speed = 6.0f;

        dengine_lighting_apply_pointlight(&pLight, stdshdr);

        if(dengine_input_get_key('W'))
            pLight.position[2] -= delta * speed;

        if(dengine_input_get_key('S'))
            pLight.position[2] += delta * speed;

        if(dengine_input_get_key('A'))
            pLight.position[0] -= delta * speed;

        if(dengine_input_get_key('D'))
            pLight.position[0] += delta * speed;

        if(dengine_input_get_key('E'))
            pLight.position[1] += delta * speed;

        if(dengine_input_get_key('C'))
            pLight.position[1] -= delta * speed;

        glm_mat4_identity(model);
        glm_translate(model, pLight.position);
        dengine_shader_set_mat4(dftshdr, "model", model[0]);
        dengine_draw_primitive(&axis, dftshdr);

        denginegui_text(10, 10, GL, NULL);

        denginegui_text(10, 10 + fontsz, fps, yellow);

        denginegui_text(10, 10 + 3 * fontsz, "WASD = MOVE LIGHT, EC = UP/DOWN", NULL);

        dengine_update();
    }
    free(prtbuf);
    free(stdshdr);
    free(dftshdr);
    dengine_material_destroy(&plane_mat);

    dengine_terminate();
    return 0;
}
