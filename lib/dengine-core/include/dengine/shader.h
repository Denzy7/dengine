#ifndef SHADER_H
#define SHADER_H


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


#include <stdint.h> //uint32

typedef struct Shader
{
    char* vertex_code;
    char* fragment_code;

    uint32_t vertex_id;
    uint32_t fragment_id;

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


#ifdef __cplusplus
}
#endif // __cplusplus


#endif // SHADER_H



