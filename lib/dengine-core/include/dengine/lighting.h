#ifndef LIGHTING_H
#define LIGHTING_H

#include <stdint.h> //uint32

#include "dengine/framebuffer.h" //fbo's
#include "dengine/shader.h" //set_mat4, bind_blocks?
#include "dengine/primitive.h" //draw

typedef enum
{
    DENGINE_LIGHT_DIR,
    DENGINE_LIGHT_POINT,
    DENGINE_LIGHT_SPOT,
}LightType;

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
    int invisiblemesh;

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
    float shadow_ortho;

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
    float direction[4];
    PointLight pointLight;
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

typedef void* Light;

#ifdef __cplusplus
extern "C" {
#endif

int dengine_lighting_init(const uint32_t n_PL, const uint32_t n_SL);

void dengine_lighting_terminate();


void dengine_lighting_light_setup(LightType type, const Light light);

void dengine_lighting_light_apply(LightType type, const Light light, const Shader* shader);

void dengine_lighting_light_shadow_draw(LightType type, Light light, const Shader* shader, const Primitive* primitive, const float* modelmtx);

//Shadow op

void dengine_lighting_shadowop_clear(const ShadowOp* shadowop);

void dengine_lighting_shadowop_resize(uint32_t shadowmap_target, ShadowOp* shadowop, int size);

//DirLight

void dengine_lighting_setup_dirlight(DirLight* dirLight);

void dengine_lighting_apply_dirlight(const DirLight* dirLight, const Shader* shader);

void dengine_lighting_shadow_dirlight_draw(DirLight* dirLight, const Shader* shader, const Primitive* primitive, const float* modelmtx);

//PointLight

void dengine_lighting_setup_pointlight(PointLight* pointLight);

void dengine_lighting_apply_pointlight(const PointLight* pointLight, const Shader* shader);

void dengine_lighting_shadow_pointlight_draw(PointLight* pointLight, const Shader* shader, const Primitive* primitive, const float* modelmtx);

//Spotlight

void dengine_lighting_setup_spotlight(SpotLight* spotLight);

void dengine_lighting_apply_spotlight(const SpotLight* spotLight, const Shader* shader);

void dengine_lighting_shadow_spotlight_draw(SpotLight* spotLight, const Shader* shader, const Primitive* primitive, const float* modelmtx);


Lighting* dengine_lighting_get();

#ifdef __cplusplus
}
#endif

#endif // LIGHTING_H
