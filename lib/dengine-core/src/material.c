#include "dengine/material.h"
#include "dengine/loadgl.h"

#include "dengine-utils/logging.h"

#include <string.h>


void dengine_material_setup(Material* material)
{
    memset(material, 0, sizeof(Material));
}

void dengine_material_set_shader_color(Shader* shader, Material* material)
{
    material->shader_color = *shader;
}

void dengine_material_set_shader_shadow(Shader* shader, Material* material)
{
    material->shader_shadow = *shader;
}

void dengine_material_set_shader_shadow3d(Shader* shader, Material* material)
{
    material->shader_shadow3d = *shader;
}

void dengine_material_add_texture(uint32_t target, Texture* texture, const char* sampler, Material* material)
{
    int loc = glGetUniformLocation(material->shader_color.program_id, sampler); DENGINE_CHECKGL;
    if(loc == -1)
    {
        dengineutils_logging_log("ERROR::MATERIAL::Cannot find sampler %s in shader. Adding failed", sampler);
    }else
    {
        material->textures[material->textures_count] = *texture;
        material->textures_targets[material->textures_count] = target;
        dengine_shader_set_int(&material->shader_color, sampler, material->textures_count);
        material->textures_count++;
    }
}

void dengine_material_use(Material* material)
{

    if (material) {
        for (int i = 0; i < material->textures_count; i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            if (material->textures[i].texture_id != 0)
                dengine_texture_bind(material->textures_targets[i], &material->textures[i]);
        }
    }else
    {
        for (int i = 0; i < 16; i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            dengine_texture_bind(GL_TEXTURE_CUBE_MAP, NULL);
            dengine_texture_bind(GL_TEXTURE_2D, NULL);
        }
    }



}
