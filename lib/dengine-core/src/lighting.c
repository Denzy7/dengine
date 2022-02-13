#include "dengine/lighting.h"

#include "dengine/loadgl.h" //gl
#include "dengine/draw.h" //draw
#include "dengine/window.h" //get w,h

#include <string.h> //memset
Lighting lighting;

#include <stdlib.h> //malloc
#include <assert.h> //assert =

#include <cglm/cglm.h> //mat, proj
const char* LightOpShaderStr=
        "int enable;\n"

        "vec4 ambient;\n"
        "vec4 diffuse;\n"
        "vec4 specular;\n"

        "float strength;\n";

#define LIGHTOP_SHADERBLOCK LightOpShaderStr


void _dengine_lighting_shadowop_setup(uint32_t shadowmap_target, ShadowOp* shadowop);

void _dengine_lighting_shadowop_setup(uint32_t shadowmap_target, ShadowOp* shadowop)
{
    Texture depth;
    memset(&depth, 0, sizeof(Texture));
    depth.height = shadowop->shadow_map_size;
    depth.width = shadowop->shadow_map_size;
    depth.format = GL_DEPTH_COMPONENT;
    depth.internal_format = GL_DEPTH_COMPONENT;
    depth.type = GL_FLOAT;
    depth.filter_min = GL_LINEAR;

    dengine_texture_gen(1, &depth);
    dengine_texture_bind(shadowmap_target, &depth);
    dengine_texture_data(shadowmap_target, &depth);
    dengine_texture_set_params(shadowmap_target, &depth);

    dengine_framebuffer_gen(1, &shadowop->shadow_map);
    dengine_framebuffer_bind(GL_FRAMEBUFFER, &shadowop->shadow_map);
    if(shadowmap_target == GL_TEXTURE_2D)
        dengine_framebuffer_attach2D(DENGINE_FRAMEBUFFER_DEPTH, &depth, &shadowop->shadow_map);
    else
        dengine_framebuffer_attach(DENGINE_FRAMEBUFFER_DEPTH, &depth, &shadowop->shadow_map);
    
    glReadBuffer(GL_NONE);
    glDrawBuffer(GL_NONE);

    dengine_texture_bind(shadowmap_target, NULL);
    dengine_framebuffer_bind(GL_FRAMEBUFFER, NULL);

    shadowop->near_shadow = 0.01f;
    shadowop->far_shadow = 25.0f;
}

int dengine_lighting_init(const uint32_t n_PL, const uint32_t n_SL)
{
    assert(n_PL > 0);
    assert(n_SL > 0);

    memset(&lighting, 0, sizeof(Lighting));

    lighting.n_pLights = n_PL;
    lighting.n_sLights = n_SL;

    lighting.pLights = malloc(sizeof(PointLight) * n_PL);
    lighting.sLights = malloc(sizeof(SpotLight) * n_SL);

    return lighting.sLights && lighting.pLights;
}

Lighting* dengine_lighting_get()
{
    return &lighting;
}

void dengine_lighting_setup_dirlight(DirLight* dirLight)
{
    if(dirLight->shadow.enable)
    {
        _dengine_lighting_shadowop_setup(GL_TEXTURE_2D, &dirLight->shadow);

        memset(dirLight->shadow_projview, 0, sizeof(dirLight->shadow_projview));
    }
}

void dengine_lighting_shadow_dirlight_draw(DirLight* dirLight, Shader* shader, Primitive* primitive, float* modelmtx)
{
    dengine_framebuffer_bind(GL_FRAMEBUFFER, &dirLight->shadow.shadow_map);

    //Guard for no shadow
    if(!dirLight->shadow.enable)
    {
        dengine_framebuffer_bind(GL_FRAMEBUFFER, NULL);
        return;
    }

    float ortho_size = 10.0f;
    mat4 proj, view;
    mat4 projview;

    glm_ortho(-ortho_size, ortho_size, -ortho_size, ortho_size, dirLight->shadow.near_shadow, dirLight->shadow.far_shadow, proj);

    vec3 eye = {dirLight->position[0], dirLight->position[1], dirLight->position[2]};
    vec3 ctr = {0.0f, 0.0f, 0.0f};
    vec3 up = {0.0f, 1.0f, 0.0};

    glm_lookat(eye, ctr, up, view);
    glm_mat4_mul(proj, view, projview);

    dengine_shader_set_mat4(shader, "projview", projview[0]);
    dengine_shader_set_mat4(shader, "model", modelmtx);

    //printf("%d %u\n", dirLight->shadow.shadow_map_size, dirLight->shadow.shadow_map.depth.texture_id);
    glViewport(0, 0, dirLight->shadow.shadow_map_size, dirLight->shadow.shadow_map_size);

    dengine_draw_primitive(primitive, shader);

    int h, w;
    dengine_window_get_window_dim(&w, &h);
    glViewport(0, 0, w, h);
    dengine_framebuffer_bind(GL_FRAMEBUFFER, NULL);

    memcpy(dirLight->shadow_projview, projview[0], sizeof(mat4));
}

void dengine_lighting_shadow_dirlight_clear(DirLight* dirLight)
{
    dengine_framebuffer_bind(GL_FRAMEBUFFER, &dirLight->shadow.shadow_map);
    glClear(GL_DEPTH_BUFFER_BIT);
    dengine_framebuffer_bind(GL_FRAMEBUFFER, NULL);
}

void dengine_lighting_apply_dirlight(DirLight* dirLight, Shader* shader)
{
    if(dirLight->shadow.enable)
    {

    }
}

int dengine_lighting_patch(Shader* shader)
{
    return 0;
}

void dengine_lighting_terminate()
{
    if(lighting.pLights)
        free(lighting.pLights);

    if(lighting.sLights)
        free(lighting.sLights);
}
