#ifndef MATERIAL_H
#define MATERIAL_H

#include <stddef.h> //size_t

#include "shader.h"
#include "texture.h"

typedef struct Material
{
    Shader shader;
    Shader shader_shadow;
    Shader shader_shadow_3d;

    Texture* textures;
    size_t textures_count;

    float specular;
}Material;

#endif // MATERIAL_H
