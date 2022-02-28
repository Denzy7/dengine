/*! \file camera.h
 *  Execute camera operations
 */

#ifndef CAMERA_H
#define CAMERA_H

#include "dengine/shader.h"

/*! \struct Camera
 *  Camera struct with camera data
 */
typedef struct
{
    float projection_mat[16]; /*!< Projection 4x4 matrix */
    float view_mat[16]; /*!< View 4x4 matrix */
    float uview_mat[9]; /*!< Untranslated view 3x3 matrix */

    float position[3]; /*!< X, Y, Z position */

    float fov; /*!< Field of view in degrees */
    float near; /*!< Near clipping plane */
    float far; /*!< Far clipping plane */
} Camera;

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Setup a camera. Assigns some default values
 * \param camera Camera to setup
 */
void dengine_camera_setup(Camera* camera);

/*!
 * \brief Apply a perspective projection using aspect to camera
 * \param aspect Aspect ratio of view frustrum. Typically window width / window height
 * \param camera Camera to apply prespective projection
 */
void dengine_camera_project_perspective(float aspect, Camera* camera);

/*!
 * \brief Generate a view matrix looking at a specific target
 * \param target An array of 3 float describing the X,Y and Z or the target
 * \param camera Camera to use
 */
void dengine_camera_lookat(float* target, Camera* camera);

/*!
 * \brief Apply matrices to a shader
 *
 * This functions sets the uniforms in the shader to the camera values and/or updates
 * uniform buffers if supported by the GL
 *
 * \param shader Shader to apply to
 * \param camera Camera to use
 */
void dengine_camera_apply(Shader* shader, Camera* camera);

#ifdef __cplusplus
}
#endif


#endif // CAMERA_H
