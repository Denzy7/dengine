#include "dengine/material.h"
#include "dengine/loadgl.h"

#include "dengine-utils/logging.h"
#include "dengine-utils/vtor.h"

#include <string.h> //memset
#include <stdlib.h> //malloc
#include <stdio.h>

void dengine_material_setup(Material* material)
{
    memset(material, 0, sizeof(Material));
    material->white=dengine_texture_new_white(256,256);
    material->normalmap = dengine_texture_new_normalmap(256, 256);
}

void dengine_material_set_shader_color(Shader* shader, Material* material)
{
    material->shader_color = *shader;

    int count, max_uniform_ln, size;
    uint32_t type;
    glGetProgramiv(shader->program_id, GL_ACTIVE_UNIFORMS, &count); DENGINE_CHECKGL;
    glGetProgramiv(shader->program_id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_uniform_ln); DENGINE_CHECKGL;
    char* uniform_name = malloc(max_uniform_ln);
    
    vtor tex;
    vtor_create(&tex, sizeof (int));

    for (int i = 0; i < count; i++)
    {
        glGetActiveUniform(shader->program_id, i, max_uniform_ln, NULL, &size, &type, uniform_name); DENGINE_CHECKGL;
        if(type == GL_SAMPLER_2D || type == GL_SAMPLER_CUBE)
        {
            vtor_pushback(&tex, &i);
        }
    }
    
    MaterialTexture* textures = malloc(tex.count * sizeof (MaterialTexture));
    memset(textures, 0, tex.count * sizeof (MaterialTexture));

    int* tex_idx = tex.data;
    for (size_t i = 0; i < tex.count; i++) {
        glGetActiveUniform(shader->program_id, tex_idx[i], max_uniform_ln, NULL, &size, &type, uniform_name); DENGINE_CHECKGL;
        if (type == GL_SAMPLER_2D) {
            textures[i].target = GL_TEXTURE_2D;
            //guess if its a normalmap
            if(strstr(uniform_name, "normal"))
                textures[i].texture = *material->normalmap;
            else
                textures[i].texture = *material->white;
        }else if (type == GL_SAMPLER_CUBE) {
            textures[i].target = GL_TEXTURE_CUBE_MAP;
        }
        textures[i].sampler = strdup(uniform_name);
        dengine_shader_set_int(shader, uniform_name, i);
    }

    material->textures = textures;
    material->textures_count = tex.count;

    vtor_free(&tex);
    free(uniform_name);
}

void dengine_material_set_shader_shadow(Shader* shader, Material* material)
{
    material->shader_shadow = *shader;
}

void dengine_material_set_shader_shadow3d(Shader* shader, Material* material)
{
    material->shader_shadow3d = *shader;
}

int dengine_material_set_texture(Texture* texture, const char* sampler, Material* material)
{
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

void dengine_material_use(Material* material)
{
    if (material) {
        for (size_t i = 0; i < material->textures_count; i++) {
            glActiveTexture(GL_TEXTURE0 + i); DENGINE_CHECKGL;
            dengine_texture_bind(material->textures[i].target, &material->textures[i].texture);
        }
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
    if(material->destroyed)
        return;

    if (material && material->textures) {
        for (size_t i = 0; i < material->textures_count; i++) {
            free(material->textures[i].sampler);
        }
        free(material->textures);
    }
    if (material)
    {
        dengine_texture_free_data(material->white);
        dengine_texture_destroy(1,material->white);
        free(material->white);

        dengine_texture_free_data(material->normalmap);
        dengine_texture_destroy(1,material->normalmap);
        free(material->normalmap);
    }

    material->destroyed = 1;
}

Texture* dengine_material_get_texture(const char* sampler, Material* material)
{
    if (!material || !material->textures)
        return NULL;

    Texture* find = NULL;

    for (size_t i = 0; i < material->textures_count; i++) {
        if (!strcmp(sampler, material->textures[i].sampler)) {
            find = &material->textures[i].texture;
        }
    }
    return find;
}
