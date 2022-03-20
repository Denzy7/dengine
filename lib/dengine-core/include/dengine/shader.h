/*! \file shader.h
 *  Execute shader operations
 */


#ifndef SHADER_H
#define SHADER_H


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


#include <stdint.h> //uint32

/*! \struct Shader
 *  Code that runs when a Primitive is drawn
 *
 *  A shader is the bread and butter of modern computer graphics. It loads code which will execute
 *  when a fragment is drawn to the screen and how it appears
 *
 */
typedef struct
{
    char* vertex_code;
    char* fragment_code;
    char* geometry_code;

    uint32_t vertex_id;
    uint32_t fragment_id;
    uint32_t geometry_id;

    uint32_t program_id;

    int linking_succeeded;
} Shader;

void dengine_shader_create(Shader* shader);

void dengine_shader_destroy(Shader* shader);

int dengine_shader_setup(Shader* shader);

int dengine_shader_link(Shader* shader);

void dengine_shader_use(const Shader* shader);

void dengine_shader_set_mat4(const Shader* shader, const char* name, float* value);

void dengine_shader_set_vec3(const Shader* shader, const char* name, float* value);

void dengine_shader_set_vec4(const Shader* shader, const char* name, float* value);

void dengine_shader_set_float(const Shader* shader, const char* name, float value);

void dengine_shader_set_int(const Shader* shader, const char* name, int value);

Shader* dengine_shader_new_shader_standard();

Shader* dengine_shader_new_shader_default();

#ifdef __cplusplus
}
#endif // __cplusplus


#endif // SHADER_H



