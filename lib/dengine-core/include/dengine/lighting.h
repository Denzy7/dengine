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

/*TODO: configure file from cmake so values are automatically changed */
#define MAX_POINT 4
#define MAX_SPOT 4

typedef struct LightOp
{
    int enable;

    float ambient[4];
    float diffuse[4];
    float specular[4];

    float strength;


    char __std140pad[8];
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
    char __std140pad[8];
}ShadowOp;

typedef struct DirLight
{
    float direction[4]; /*!< direction here is a bit incorrect. its more of direection + position,
                          yes, a dirLight technically can't have a position but this is very useful
                          when we want to fit a shadowmap that perfectly fits a scene and the camera
                          keeps a good distance so as not to show articats*/
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
    PointLight* pLights[MAX_POINT];
    SpotLight* sLights[MAX_POINT];

    uint32_t n_pLights, n_sLights;
}Lighting;

/* type is inferred on usage
 * pass the pointer to the light*/
typedef void Light;

#ifdef __cplusplus
extern "C" {
#endif

int dengine_lighting_init(const uint32_t n_PL, const uint32_t n_SL);

void dengine_lighting_terminate();


void dengine_lighting_light_setup(const LightType type, Light* light);

void dengine_lighting_light_apply(const LightType type, const Light* light, const Shader* shader);

LightOp* dengine_lighting_light_get_lightop(LightType type, Light* light);

ShadowOp* dengine_lighting_light_get_shadowop(LightType type, Light* light);

void dengine_lighting_light_shadow_draw(LightType type, Light* light, const Shader* shader, const Primitive* primitive, const float* modelmtx);

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
