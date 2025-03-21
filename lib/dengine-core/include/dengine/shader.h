/*! \file shader.h
 *  Execute shader operations
 */


#ifndef SHADER_H
#define SHADER_H

#include <stdint.h> //uint32


/*! \enum
 * Hint the upstream scene queuer which queue
 * to store any mesh that uses this shader. defaults
 * to the opaque queue. see how queues work in dengine-scene/queue.h
 */
typedef enum
{
    DENGINE_SHADER_SCENEQUEUER_SHADERHINT_OPAQUE,
    DENGINE_SHADER_SCENEQUEUER_SHADERHINT_TRANSPARENT,
    DENGINE_SHADER_SCENEQUEUER_SHADERHINT_UI,
}SceneQueuerShaderHint;
/*! \struct Shader
 *  Code that runs when a Primitive is drawn
 *
 *  A shader is the bread and butter of modern computer graphics. It loads code which will execute
 *  when a fragment is drawn to the screen and how it appears
 *
 */
typedef struct
{
    const char* vertex_code;
    const char* fragment_code;
    const char* geometry_code;
    const char* cached_name;

    uint32_t vertex_id;
    uint32_t fragment_id;
    uint32_t geometry_id;

    uint32_t program_id;

    int linking_succeeded;
    SceneQueuerShaderHint hint; /*!< hint the scene queuer where to place meshes with this shader. by default, all shaders are queued to 
    MESH_OPAQUE. If you want to change it, do so before queuer runs */
} Shader;

/*! \struct StandardShader
 *  Built in standard shaders
 */
typedef enum
{
    DENGINE_SHADER_STANDARD, /*!< 3D standard lit shader that just works */
    DENGINE_SHADER_DEFAULT, /*!< 3D basic shader with a single color uniform */
    DENGINE_SHADER_SHADOW2D, /*!< 3D shader for 2D shadow mapping */
    DENGINE_SHADER_SHADOW3D, /*!< 3D shader for 3D shadow mapping. GL/ES 3.2+ only */
    DENGINE_SHADER_GUI_TEXT, /*!< 2D shader for GUI quad, for rendering text */
    DENGINE_SHADER_GUI_PANEL, /*!< 2D shader for GUI quad, for rendering panel */
    DENGINE_SHADER_GUI_DISCARD, /*!< 2D shader for GUI quad, that discard fragment by threshold */
    DENGINE_SHADER_DEBUG_NORMALS, /*!< 3D shader that outputs normals as color */
    DENGINE_SHADER_SKYBOXCUBE, /*!< 3D shader when using a cubemap skybox */
    DENGINE_SHADER_SKYBOX2D, /*!< 3D shader when using a 2D skybox (hdri map) */

    DENGINE_SHADER_COUNT
}StandardShader;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void dengine_shader_create(Shader* shader);

void dengine_shader_destroy(Shader* shader);

int dengine_shader_setup(Shader* shader);

void* dengine_shader_get_binary(Shader* shader, int* length);

int dengine_shader_set_binary(Shader* shader, void* binary, int length);

int dengine_shader_link(Shader* shader);

void dengine_shader_use(const Shader* shader);

void dengine_shader_set_mat4(const Shader* shader, const char* name, const float* value);

void dengine_shader_set_vec3(const Shader* shader, const char* name, const float* value);

void dengine_shader_set_vec4(const Shader* shader, const char* name, const float* value);

void dengine_shader_set_float(const Shader* shader, const char* name, const float value);

void dengine_shader_set_int(const Shader* shader, const char* name, const int value);

void dengine_shader_current_set_mat4(const char* name, const float* value);

void dengine_shader_current_set_vec3(const char* name, const float* value);

void dengine_shader_current_set_vec4(const char* name, const float* value);

void dengine_shader_current_set_float(const char* name, const float value);

void dengine_shader_current_set_int(const char* name, const int value);

int dengine_shader_make_standard(StandardShader stdshader, Shader* shader);

void dengine_shader_set_shadercache(int state);

const uint32_t dengine_shader_sampler2target(const uint32_t sampler);

#ifdef __cplusplus
}
#endif // __cplusplus


#endif // SHADER_H



