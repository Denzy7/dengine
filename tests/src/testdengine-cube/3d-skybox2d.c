#include <dengine/dengine.h>

void draw_axis(Primitive* axis, Shader* shader)
{
    float color[3];
    for (int i = 0; i < 3; i++) {
        color[0] = i == 0 ? 1.0f : 0.0f, color[1] = i == 1 ? 1.0f : 0.0f, color[2] = i == 2 ? 1.0f : 0.0f;
        axis->offset = (void*)(i*2*sizeof (uint16_t));
        dengine_shader_set_vec3(shader, "color", color);
        dengine_draw_primitive(axis, shader);
    }
}

int main(int argc, char *argv[])
{
    DengineInitOpts* opts = dengine_init_get_opts();
    opts->window_title  = "testdengine-cube-skybox2d";
    dengine_init();

    const size_t prtbf_sz = 2048;
    char* prtbf = malloc(prtbf_sz);

    Shader sky, dft;
    dengine_shader_make_standard(DENGINE_SHADER_SKYBOX2D, &sky);
    dengine_shader_make_standard(DENGINE_SHADER_DEFAULT, &dft);

    Primitive cube, axis;
    dengine_primitive_gen_cube(&cube, &sky);
    dengine_primitive_gen_axis(&axis, &dft);
    axis.index_count = 2;

    Material cube_mat;
    dengine_material_setup(&cube_mat);
    dengine_material_set_shader_color(&sky, &cube_mat);

    Camera camera;
    dengine_camera_setup(&camera);
    camera.position[1]*= -1.0f;

    Texture equireq;
    memset(&equireq, 0, sizeof(Texture));
    /* if unsupported fallback to 8-BIT (less precision looks pretty bad after conversion 😢 ) */
    if(dengine_texture_issupprorted(GL_TEXTURE_2D, GL_FLOAT, GL_RGB, GL_RGB))
    {
        equireq.interface = DENGINE_TEXTURE_INTERFACE_FLOAT;
        equireq.type = GL_FLOAT;
    }else
    {
        equireq.interface = DENGINE_TEXTURE_INTERFACE_8_BIT;
        equireq.type = GL_UNSIGNED_BYTE;
    }
    snprintf(prtbf, prtbf_sz,
             "%s/textures/hdri/sunset.hdr",
             dengineutils_filesys_get_assetsdir());
    dengine_texture_load_file(prtbf, 1, &equireq);

    equireq.filter_min = GL_LINEAR;
    equireq.format = GL_RGB;
    equireq.internal_format = GL_RGB;
    dengine_texture_gen(1, &equireq);
    dengine_texture_bind(GL_TEXTURE_2D, &equireq);
    dengine_texture_data(GL_TEXTURE_2D, &equireq);
    dengine_texture_set_params(GL_TEXTURE_2D, &equireq);
    dengine_texture_bind(GL_TEXTURE_2D, NULL);
    dengine_texture_free_data(&equireq);

    dengine_material_set_texture(&equireq, "eqireqMap", &cube_mat);

    //disable culling
    glDisable(GL_CULL_FACE);
    glLineWidth(4.0);

    mat4 model;

    while(dengine_update())
    {
        double delta = dengineutils_timer_get_delta() / 1000.0;
        double speed = 5;

        if(dengine_input_get_key('A'))
            camera.position[0] -= speed * delta;

        if(dengine_input_get_key('D'))
            camera.position[0] += speed * delta;

        if(dengine_input_get_key('W'))
            camera.position[2] -= speed * delta;

        if(dengine_input_get_key('S'))
            camera.position[2] += speed * delta;

        if(dengine_input_get_key('E'))
            camera.position[1] += speed * delta;

        if(dengine_input_get_key('C'))
            camera.position[1] -= speed * delta;

        dengine_camera_lookat(NULL, &camera);
        dengine_camera_apply(&sky, &camera);
        dengine_camera_apply(&dft, &camera);

        //GEOMETRY
        glDepthFunc(GL_LESS);

        glm_mat4_identity(model);
        dengine_shader_set_mat4(&dft, "model", model[0]);
        draw_axis(&axis, &dft);

        //&skyBOX
        glDepthFunc(GL_LEQUAL);

        dengine_material_use(&cube_mat);

        dengine_draw_primitive(&cube, &sky);

        dengine_material_use(NULL);

        //GUI
        glDepthFunc(GL_LESS);

        denginegui_text(10, 10, "USE WASD=move camera, EC = up/dowm", NULL);
    }

    dengine_material_destroy(&cube_mat);

    free(prtbf);
    dengine_terminate();
    return 0;
}
