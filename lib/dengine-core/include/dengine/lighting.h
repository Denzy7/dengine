#ifndef LIGHTING_H
#define LIGHTING_H

#include <stdint.h> //uint32

#include "dengine/framebuffer.h" //fbo's
#include "dengine/shader.h" //set_mat4, bind_blocks?
#include "dengine/primitive.h" //draw

typedef struct LightOp
{
    int enable;

    float ambient[4];
    float diffuse[4];
    float specular[4];

    float strength;
}LightOp;

typedef struct ShadowOp
{
    int enable;

    Framebuffer shadow_map;
    int shadow_map_size;

    int pcf, pcf_samples;

    float max_bias, min_bias;

    float far_shadow, near_shadow;
}ShadowOp;

typedef struct DirLight
{
    float position[4];
    float shadow_projview[16];

    LightOp light;
    ShadowOp shadow;

    float __std140_padding[3];

}DirLight;

typedef struct PointLight
{
    float position[4];

    LightOp light;
    ShadowOp shadow;

    float constant, linear, quadratic;

    float __std140_padding[1];
}PointLight;

typedef struct SpotLight
{
    float position[4];

    LightOp light;
    ShadowOp shadow;

    float constant, linear, quadratic;
    float innerCutOff, outerCutOff;

    float __std140_padding[1];
}SpotLight;

typedef struct Lighting
{
    DirLight dLight;
    PointLight* pLights;
    SpotLight* sLights;

    uint32_t n_pLights, n_sLights;
}Lighting;

int dengine_lighting_init(const uint32_t n_PL, const uint32_t n_SL);

void dengine_lighting_terminate();


void dengine_lighting_setup_dirlight(DirLight* dirLight);

void dengine_lighting_apply_dirlight(DirLight* dirLight, Shader* shader);

void dengine_lighting_shadow_dirlight_draw(DirLight* dirLight, Shader* shader, Primitive* primitive, float* modelmtx);

void dengine_lighting_shadow_dirlight_clear(DirLight* dirLight);


int dengine_lighting_patch(Shader* shader);



Lighting* dengine_lighting_get();


#endif // LIGHTING_H
