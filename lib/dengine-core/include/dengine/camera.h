#ifndef CAMERA_H
#define CAMERA_H

#include "dengine/shader.h"

typedef struct Camera
{
    float projection_mat[16];
    float view_mat[16];
    float uview_mat[12];

    float position[3];

    float fov, near, far;
} Camera;

#ifdef __cplusplus
extern "C" {
#endif

void dengine_camera_setup(Camera* camera);

void dengine_camera_project_perspective(float aspect, Camera* camera);

void dengine_camera_lookat(float* target, Camera* camera);

void dengine_camera_apply(Shader* shader, Camera* camera);

#ifdef __cplusplus
}
#endif


#endif // CAMERA_H
