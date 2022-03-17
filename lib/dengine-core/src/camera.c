#include "camera.h"

#include "dengine/macros.h"
#include "dengine/loadgl.h"
#include "dengine/window.h"

#include <cglm/cglm.h> //glm_proj
#include <string.h>    //memcpy

float _target_zero[3] = {0.0f, 0.0f, 0.0f};
float _default_distance = 7.0f;
float _default_clearcol[4] = {1.0f, 0.5f, 0.3f, 1.0};

static const char* possible_projmat[]=
{
  "projection", "camera.projection", "proj",
};

static const char* possible_viewmat[]=
{
  "view", "camera.view"
};

static const char* possible_campos[]=
{
  "ViewPos", "camera.position"
};

void dengine_camera_setup(Camera* camera)
{
    memset(camera, 0, sizeof (Camera));
    camera->fov = 60.0f;
    camera->near = 0.01f;
    camera->far = 100.0f;
    for (int i = 0; i < 3; i++) {
        camera->position[i] = _default_distance;
    }
    memcpy(camera->clearcolor, _default_clearcol, sizeof (_default_clearcol));
    int w,h;
    dengine_window_get_window_dim(&w, &h);
    camera->render_height = h;
    camera->render_width = w;

    dengine_camera_project_perspective( (float)w / (float)h, camera);
    dengine_camera_lookat(NULL, camera);
}

void dengine_camera_project_perspective(float aspect, Camera* camera)
{
    mat4 proj;
    glm_perspective(glm_rad(camera->fov), aspect, camera->near, camera->far, proj);
    memcpy(camera->projection_mat, proj, sizeof(camera->projection_mat));
}

void dengine_camera_lookat(float* target, Camera* camera)
{
    mat4 view;
    mat3 uview;
    vec3 up = {0.0f, 1.0f, 0.0f};

    float* _target = target;
    if (!_target) {
        _target = _target_zero;
    }

    glm_lookat(camera->position, _target, up, view);

    memcpy(camera->view_mat, view, sizeof(camera->view_mat));
    memcpy(camera->uview_mat, view, sizeof(camera->uview_mat));
}

void dengine_camera_apply(Shader* shader, Camera* camera)
{
    for (size_t i = 0; i < DENGINE_ARY_SZ(possible_projmat); i++) {
        dengine_shader_set_mat4(shader, possible_projmat[i], camera->projection_mat);
    }

    for (size_t i = 0; i < DENGINE_ARY_SZ(possible_viewmat); i++) {
        dengine_shader_set_mat4(shader, possible_viewmat[i], camera->view_mat);
    }

    for (size_t i = 0; i < DENGINE_ARY_SZ(possible_campos); i++) {
        dengine_shader_set_vec3(shader, possible_campos[i], camera->position);
    }
}

void dengine_camera_set_rendermode(CameraRenderMode mode, Camera* camera)
{
    if (mode == DENGINE_CAMERA_RENDER_FOWARD) {
        Texture rgba_depth[2];
        memset(&rgba_depth,0,sizeof (rgba_depth));

        dengine_texture_gen(2, rgba_depth);
        Texture* rgba =  &rgba_depth[0];
        Texture* depth =  &rgba_depth[1];  //Use renderbuffer?

        //rgba
        rgba->format = GL_RGBA;
        rgba->internal_format = GL_RGBA;
        rgba->width =  camera->render_width;
        rgba->height = camera->render_height;
        rgba->filter_mag = GL_LINEAR;
        rgba->filter_min = GL_LINEAR;
        rgba->type = GL_UNSIGNED_BYTE;

        dengine_texture_bind(GL_TEXTURE_2D, rgba);
        dengine_texture_data(GL_TEXTURE_2D, rgba);
        dengine_texture_set_params(GL_TEXTURE_2D, rgba);

        //depth
        depth->format = GL_DEPTH_COMPONENT;
        depth->internal_format = GL_DEPTH_COMPONENT;
        depth->width = camera->render_width;
        depth->height = camera->render_height;
        depth->type = GL_UNSIGNED_BYTE;

        dengine_texture_bind(GL_TEXTURE_2D, depth);
        dengine_texture_data(GL_TEXTURE_2D, depth);
        dengine_texture_set_params(GL_TEXTURE_2D, depth);

        dengine_framebuffer_gen(1, &camera->framebuffer);
        dengine_framebuffer_bind(GL_FRAMEBUFFER, &camera->framebuffer);
        dengine_framebuffer_attach2D(DENGINE_FRAMEBUFFER_COLOR, rgba, &camera->framebuffer);
        dengine_framebuffer_attach2D(DENGINE_FRAMEBUFFER_DEPTH, depth, &camera->framebuffer);
        dengine_framebuffer_bind(GL_FRAMEBUFFER,NULL);
    }
}

void dengine_camera_use(Camera* camera)
{
    if (camera && camera->framebuffer.color->texture_id == 0)
        return;

    if (camera) {
        dengine_framebuffer_bind(GL_FRAMEBUFFER, &camera->framebuffer);
        glClearColor(
                camera->clearcolor[0],
                camera->clearcolor[1],
                camera->clearcolor[2],
                camera->clearcolor[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }else
    {
        dengine_framebuffer_bind(GL_FRAMEBUFFER, NULL);
    }
}
