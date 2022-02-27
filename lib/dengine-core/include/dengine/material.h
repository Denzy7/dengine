#ifndef MATERIAL_H
#define MATERIAL_H

#include <stddef.h> //size_t

#include "shader.h"
#include "texture.h"

typedef struct Material
{
    Shader shader_color;
    Shader shader_shadow;
    Shader shader_shadow3d;

    uint32_t textures_targets[16];
    Texture textures[16];
    size_t textures_count;

    float specular;
}Material;

#ifdef __cplusplus
extern "C" {
#endif

void dengine_material_setup(Material* material);

void dengine_material_set_shader_color(Shader* shader, Material* material);

void dengine_material_set_shader_shadow(Shader* shader, Material* material);

void dengine_material_set_shader_shadow3d(Shader* shader, Material* material);

void dengine_material_add_texture(uint32_t target, Texture* texture, const char* sampler, Material* material);

void dengine_material_use(Material* material);

#ifdef __cplusplus
}
#endif

#endif // MATERIAL_H
