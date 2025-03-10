#include "dengine/material.h"
#include "dengine/loadgl.h"

#include "dengine-utils/logging.h"
#include "dengine-utils/vtor.h"
#include "dengine-utils/debug.h"

#include <string.h> //memset
#include <stdlib.h> //malloc
#include <stdio.h>
MaterialID _nummaterials = 0;
void dengine_material_setup(Material* material)
{

    DENGINE_DEBUG_ENTER;

    memset(material, 0, sizeof(Material));
    static const float white[] = {1., 1., 1.};
    static const float normal[] = {0.5, 0.5, 1.};
    dengine_texture_make_color(8, 8, white, 3, &material->white);
    dengine_texture_make_color(8, 8, normal, 3, &material->normalmap);

    for(int i = 0; i < 3; i++)
    {
        material->properties.ambient[i] = 1.0f;
        material->properties.diffuse[i] = 1.0f;
        material->properties.specular[i] = 1.0f;
    }
    material->properties.alpha = 1.0f;
    material->properties.specular_power = 32.0f;

    material->id = _nummaterials;
    _nummaterials++;
}

void dengine_material_set_shader_color(const Shader* shader, Material* material)
{
    DENGINE_DEBUG_ENTER;

    material->shader_color = *shader;

    int count, max_uniform_ln, size, max_units;
    uint32_t type;
    glGetProgramiv(shader->program_id, GL_ACTIVE_UNIFORMS, &count); DENGINE_CHECKGL;
    glGetProgramiv(shader->program_id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_uniform_ln); DENGINE_CHECKGL;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_units); DENGINE_CHECKGL;
    char* uniform_name = malloc(max_uniform_ln);

    memset(&material->textures, 0, sizeof(material->textures));
    material->textures_count = 0;
    for (int i = 0; i < count; i++)
    {
        glGetActiveUniform(shader->program_id, i, max_uniform_ln, NULL, &size, &type, uniform_name); DENGINE_CHECKGL;
        /*printf("active: %s\n", uniform_name);*/
        if (type == GL_SAMPLER_2D || type == GL_SAMPLER_CUBE)
        {
            if(material->textures_count > max_units)
            {
                dengineutils_logging_log("WARNING::This OpenGL driver has a maximum of %d texture units."
                                         "%s will not be activated", max_units, uniform_name);
            }else if(material->textures_count > DENGINE_MAX_MATERIAL_TEXTURES)
            {
                dengineutils_logging_log("WARNING::Reached material texture limit. Recompile to increase limit");
            }else{
                MaterialTexture* texture = &material->textures[material->textures_count];
                strncpy(texture->sampler, uniform_name, sizeof(texture->sampler));
                texture->target = dengine_shader_sampler2target(type);
                if(type == GL_SAMPLER_2D)
                {
                    //assign some blank 2D textures

                    //guess if its a normalmap
                    if(strstr(uniform_name, "normal"))
                        texture->texture = material->normalmap;
                    else
                        texture->texture = material->white;
                }
                material->textures_count++;
            }
        }
    }

    //clamp to the max units
    if(material->textures_count > max_units)
        material->textures_count = max_units;

    free(uniform_name);
}

void dengine_material_set_shader_shadow(const Shader* shader, Material* material)
{
    DENGINE_DEBUG_ENTER;

    material->shader_shadow = *shader;
}

void dengine_material_set_shader_shadow3d(const Shader* shader, Material* material)
{
    DENGINE_DEBUG_ENTER;

    material->shader_shadow3d = *shader;
}

int dengine_material_set_texture(const Texture* texture, const char* sampler, Material* material)
{
    DENGINE_DEBUG_ENTER;

    int ok = 0;
    for (size_t i = 0; i < material->textures_count; i++)
    {
        if (!strcmp(sampler, material->textures[i].sampler)) {
            ok = 1;
            material->textures[i].texture = *texture;
        }
    }
    return ok;
}

void dengine_material_use(const Material* material)
{
    DENGINE_DEBUG_ENTER;

    if (material) {
        dengine_shader_use(&material->shader_color);
        for (size_t i = 0; i < material->textures_count; i++) {
            glActiveTexture(GL_TEXTURE0 + i); DENGINE_CHECKGL;
            dengine_texture_bind(material->textures[i].target, &material->textures[i].texture);
            dengine_shader_current_set_int(material->textures[i].sampler, i);
        }
        dengine_shader_current_set_vec3("material.color.diffuse", material->properties.diffuse);
        dengine_shader_current_set_vec3("material.color.specular", material->properties.specular);
        dengine_shader_current_set_vec3("material.color.ambient", material->properties.ambient);
        dengine_shader_current_set_float("material.specular_power", material->properties.specular_power);
        dengine_shader_current_set_float("material.alpha", material->properties.alpha);
    }else
    {
        int max_tex=0;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_tex); DENGINE_CHECKGL;
        for (int i = 0; i < max_tex; i++) {
            glActiveTexture(GL_TEXTURE0 + i); DENGINE_CHECKGL;
            dengine_texture_bind(GL_TEXTURE_CUBE_MAP, NULL);
            dengine_texture_bind(GL_TEXTURE_2D, NULL);
        }
    }
}

void dengine_material_destroy(Material* material)
{
    DENGINE_DEBUG_ENTER;

    dengine_texture_destroy(1, &material->white);
    dengine_texture_destroy(1, &material->normalmap);
}

const Texture* dengine_material_get_texture(const char* sampler, Material* material)
{
    DENGINE_DEBUG_ENTER;

    const Texture* find = NULL;

    for (size_t i = 0; i < material->textures_count; i++) {
        if (!strcmp(sampler, material->textures[i].sampler)) {
            find = &material->textures[i].texture;
        }
    }
    return find;
}
