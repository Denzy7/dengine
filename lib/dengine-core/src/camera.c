#include "camera.h"

#include "dengine/macros.h"

#include <cglm/cglm.h> //glm_proj
#include <string.h>    //memcpy

float _target_zero[3] = {0.0f, 0.0f, 0.0f};
float _default_distance = 7.0f;

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
