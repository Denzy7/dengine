#include "camera.h"

#include <cglm/cglm.h> //glm_proj
#include <string.h>    //memcpy

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

    glm_lookat(camera->position, target, up, view);

    memcpy(camera->view_mat, view, sizeof(camera->view_mat));
    memcpy(camera->uview_mat, view, sizeof(camera->uview_mat));
}
