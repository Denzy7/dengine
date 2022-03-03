#include "dengine/lighting.h"

#include "dengine/loadgl.h" //gl
#include "dengine/draw.h" //draw
#include "dengine/window.h" //get w,h
#include "dengine/macros.h" //arr_sz

#include <string.h> //memset
Lighting lighting;

#include <stdlib.h> //malloc
#include <assert.h> //assert =

#include <cglm/cglm.h> //mat, proj

static char prtbuf[1024];

static vec3 centers_3d[] = {
    {1.0f, 0.0f, 0.0f},
    {-1.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f},
    {0.0f, -1.0f, 0.0f},
    {0.0f, 0.0f, 1.0f},
    {0.0f, 0.0f, -1.0f}
};

static vec3 ups_3d[] = {
    {0.0f, -1.0f, 0.0f},
    {0.0f, -1.0f, 0.0f},
    {0.0f, 0.0f, 1.0f},
    {0.0f, 0.0f, -1.0f},
    {0.0f, -1.0f, 0.0f},
    {0.0f, -1.0f, 0.0f},
};

static const float default_ambient[3]=
{
    .1f,.1f,.1f
};

static const float default_specular[3]=
{
    .75f,.75f,.75f
};

static const float default_diffuse[3]=
{
    .75f,.75f,.75f
};

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
    depth.type = GL_UNSIGNED_BYTE;
    depth.filter_min = GL_NEAREST;
    depth.filter_mag = GL_NEAREST;
    depth.wrap = GL_CLAMP_TO_EDGE;

    dengine_texture_gen(1, &depth);
    dengine_texture_bind(shadowmap_target, &depth);
    if(shadowmap_target == GL_TEXTURE_2D){
        dengine_texture_data(shadowmap_target, &depth);
    }else if (shadowmap_target == GL_TEXTURE_CUBE_MAP)
    {
        for(uint32_t i = 0; i < 6; i++)
        {
            uint32_t face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
            dengine_texture_data(face, &depth);
        }
    }
    dengine_texture_set_params(shadowmap_target, &depth);

    dengine_framebuffer_gen(1, &shadowop->shadow_map);
    dengine_framebuffer_bind(GL_FRAMEBUFFER, &shadowop->shadow_map);
    if(shadowmap_target == GL_TEXTURE_2D)
        dengine_framebuffer_attach2D(DENGINE_FRAMEBUFFER_DEPTH, &depth, &shadowop->shadow_map);
    else
        dengine_framebuffer_attach(DENGINE_FRAMEBUFFER_DEPTH, &depth, &shadowop->shadow_map);

    GLenum none[] = {GL_NONE};
    glReadBuffer(GL_NONE); DENGINE_CHECKGL;
    glDrawBuffers(1, none); DENGINE_CHECKGL;

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

void dengine_lighting_shadowop_clear(ShadowOp* shadowop)
{
    if (shadowop->shadow_map.depth.texture_id == 0)
        return;

    dengine_framebuffer_bind(GL_FRAMEBUFFER, &shadowop->shadow_map);
    glClear(GL_DEPTH_BUFFER_BIT); DENGINE_CHECKGL;
    dengine_framebuffer_bind(GL_FRAMEBUFFER, NULL);
}

void dengine_lighting_setup_dirlight(DirLight* dirLight)
{
    if(dirLight->shadow.enable)
    {
        _dengine_lighting_shadowop_setup(GL_TEXTURE_2D, &dirLight->shadow);

        memset(dirLight->shadow_projview, 0, sizeof(dirLight->shadow_projview));
    }
    static const float default_dirlight_pos[3] =
    {
      -4.0f, 5.0f, 6.0f
    };

    memcpy(dirLight->position, default_dirlight_pos, sizeof (default_dirlight_pos));
    memcpy(dirLight->light.ambient, default_ambient, sizeof (default_ambient));
    memcpy(dirLight->light.diffuse, default_diffuse, sizeof (default_diffuse));
    memcpy(dirLight->light.specular, default_specular, sizeof (default_specular));

    dirLight->shadow.pcf = 0;
    dirLight->shadow.pcf_samples = 4;

    dirLight->light.strength = 0.55;
}

void dengine_lighting_shadow_dirlight_draw(DirLight* dirLight, Shader* shader, Primitive* primitive, float* modelmtx)
{
    //Guard for no shadow
    if(!dirLight->shadow.enable || !dirLight->shadow.shadow_map.depth.texture_id) {
        return;
    }

    dengine_framebuffer_bind(GL_FRAMEBUFFER, &dirLight->shadow.shadow_map);

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

void dengine_lighting_apply_dirlight(DirLight* dirLight, Shader* shader)
{
    static const char* possible_pos[]=
    {
        "lightDir","dLight.position"
    };

    for (size_t i = 0; i < DENGINE_ARY_SZ(possible_pos); i++) {
        dengine_shader_set_vec3(shader, possible_pos[i], dirLight->position);
    }

    dengine_shader_set_vec3(shader, "dLight.light.ambient", dirLight->light.ambient);
    dengine_shader_set_vec3(shader, "dLight.light.diffuse", dirLight->light.diffuse);
    dengine_shader_set_vec3(shader, "dLight.light.specular", dirLight->light.specular);
    dengine_shader_set_float(shader, "dLight.light.strength", dirLight->light.strength);
    dengine_shader_set_float(shader, "dLight.shadow.max_bias", dirLight->shadow.max_bias);
    dengine_shader_set_int(shader, "dLight.shadow.enable", dirLight->shadow.enable);
    dengine_shader_set_int(shader, "dLight.shadow.pcf", dirLight->shadow.pcf);
    dengine_shader_set_int(shader, "dLight.shadow.pcf_samples", dirLight->shadow.pcf_samples);
    dengine_shader_set_mat4(shader, "dLight.shadow_projview", dirLight->shadow_projview);

}

void dengine_lighting_setup_pointlight(PointLight* pointLight)
{
    if(pointLight->shadow.enable)
    {
        _dengine_lighting_shadowop_setup(GL_TEXTURE_CUBE_MAP, &pointLight->shadow);
    }

    pointLight->constant = 1.0f;
    pointLight->linear  = 0.09f;
    pointLight->quadratic = 0.032f;

    pointLight->light.strength = 1.0f;

    memcpy(pointLight->light.ambient, default_ambient, sizeof(default_ambient));
    memcpy(pointLight->light.diffuse, default_diffuse, sizeof(default_diffuse));
    memcpy(pointLight->light.specular, default_specular, sizeof(default_specular));

    pointLight->shadow.far_shadow = 30.0f;
    pointLight->shadow.near_shadow = 0.1f;
}

void dengine_lighting_apply_pointlight(PointLight* pointLight, Shader* shader)
{
    size_t bufsz = sizeof (prtbuf);
    for (uint32_t i = 0; i < 1; i++) {
        snprintf(prtbuf, bufsz,"pLights[%u].position",i);
        dengine_shader_set_vec3(shader, prtbuf, pointLight->position);

        snprintf(prtbuf, bufsz,"pLights[%u].light.ambient",i);
        dengine_shader_set_vec3(shader, prtbuf, pointLight->light.ambient);

        snprintf(prtbuf, bufsz,"pLights[%u].light.diffuse",i);
        dengine_shader_set_vec3(shader, prtbuf, pointLight->light.diffuse);

        snprintf(prtbuf, bufsz,"pLights[%u].light.specular",i);
        dengine_shader_set_vec3(shader, prtbuf, pointLight->light.specular);

        snprintf(prtbuf, bufsz,"pLights[%u].constant",i);
        dengine_shader_set_float(shader, prtbuf, pointLight->constant);

        snprintf(prtbuf, bufsz,"pLights[%u].linear",i);
        dengine_shader_set_float(shader, prtbuf, pointLight->linear);

        snprintf(prtbuf, bufsz,"pLights[%u].quadratic",i);
        dengine_shader_set_float(shader, prtbuf, pointLight->quadratic);

        snprintf(prtbuf, bufsz,"pLights[%u].shadow.far_shadow",i);
        dengine_shader_set_float(shader, prtbuf, pointLight->shadow.far_shadow);

        snprintf(prtbuf, bufsz,"pLights[%u].light.strength",i);
        dengine_shader_set_float(shader, prtbuf, pointLight->light.strength);
    }

    //Compatibility until tests are upgraded
    float diffStrength[3];
    memcpy(diffStrength, pointLight->light.diffuse, sizeof(diffStrength));;

    for(int j = 0; j < 3; j++)
        diffStrength[j]*=pointLight->light.strength;

    dengine_shader_set_vec3(shader, "lightPos", pointLight->position);
    dengine_shader_set_vec3(shader, "diffuseCol", diffStrength);
    dengine_shader_set_float(shader, "constant", pointLight->constant);
    dengine_shader_set_float(shader, "linear", pointLight->linear);
    dengine_shader_set_float(shader, "quadratic", pointLight->quadratic);
    dengine_shader_set_float(shader, "shadowfar", pointLight->shadow.far_shadow);
}

void dengine_lighting_shadow_pointlight_draw(PointLight* pointLight, Shader* shader, Primitive* primitive, float* modelmtx)
{
    //Guard for no shadow
    if (!pointLight->shadow.shadow_map.depth.texture_id|| !pointLight->shadow.enable)
        return;

    dengine_framebuffer_bind(GL_FRAMEBUFFER, &pointLight->shadow.shadow_map);

    mat4 proj, view;
    mat4 projviews[6];

    vec3 pos, posdir;
    memcpy(pos, pointLight->position, sizeof(vec3));

    int shadow_map_size = pointLight->shadow.shadow_map_size;
    float aspect = (float)shadow_map_size / (float)shadow_map_size;
    float near = pointLight->shadow.near_shadow;
    float far = pointLight->shadow.far_shadow;

    glm_perspective(glm_rad(90.0f), aspect, near, far, proj);
    dengine_shader_set_mat4(shader, "model", modelmtx);

    //char matrix_str[strlen("matrices[00]")];
    char matrix_str[32];
    for(int i = 0; i < 6; i++)
    {
        glm_mat4_zero(view);
        glm_vec3_zero(posdir);
        glm_vec3_add(pos, centers_3d[i], posdir);
        glm_lookat(pos, posdir, ups_3d[i], view);
        glm_mat4_mul(proj, view, projviews[i]);

        snprintf(matrix_str, sizeof(matrix_str), "matrices[%d]", i);
        dengine_shader_set_mat4(shader, matrix_str, projviews[i][0]);
    }

    dengine_shader_set_vec3(shader, "pos", pos);
    dengine_shader_set_float(shader, "far", far);

    glViewport(0, 0, shadow_map_size, shadow_map_size);

    dengine_draw_primitive(primitive, shader);

    int h, w;
    dengine_window_get_window_dim(&w, &h);
    glViewport(0, 0, w, h);
    dengine_framebuffer_bind(GL_FRAMEBUFFER, NULL);
}

void dengine_lighting_setup_spotlight(SpotLight* spotLight)
{
    dengine_lighting_setup_pointlight(&spotLight->pointLight);

    spotLight->innerCutOff = 2.0f;
    spotLight->outerCutOff = 3.0f;

    spotLight->pointLight.light.strength = 1.f;

    float diff[3] = {1.0f, 1.0f, 1.0f};
    memcpy(spotLight->pointLight.light.diffuse, diff, sizeof(diff));

    spotLight->pointLight.shadow.far_shadow = 30.0f;
    spotLight->pointLight.shadow.near_shadow = 0.1f;
}

void dengine_lighting_apply_spotlight(SpotLight* spotLight, Shader* shader)
{
    float oCut_rad = glm_rad(45.0f - glm_clamp(spotLight->outerCutOff, 0.0f, 45.0f));
    float iCut_rad = glm_rad(45.0f - glm_clamp(spotLight->innerCutOff, 0.0f, spotLight->outerCutOff));

    size_t bufsz = sizeof (prtbuf);
    for (uint32_t i = 0; i < 1; i++) {
        snprintf(prtbuf, bufsz,"sLights[%u].pointLight.position",i);
        dengine_shader_set_vec3(shader, prtbuf, spotLight->pointLight.position);

        snprintf(prtbuf, bufsz,"sLights[%u].pointLight.light.ambient",i);
        dengine_shader_set_vec3(shader, prtbuf, spotLight->pointLight.light.ambient);

        snprintf(prtbuf, bufsz,"sLights[%u].pointLight.light.diffuse",i);
        dengine_shader_set_vec3(shader, prtbuf, spotLight->pointLight.light.diffuse);

        snprintf(prtbuf, bufsz,"sLights[%u].pointLight.light.specular",i);
        dengine_shader_set_vec3(shader, prtbuf, spotLight->pointLight.light.specular);

        snprintf(prtbuf, bufsz,"sLights[%u].direction",i);
        dengine_shader_set_vec3(shader, prtbuf, spotLight->direction);

        snprintf(prtbuf, bufsz,"sLights[%u].pointLight.constant",i);
        dengine_shader_set_float(shader, prtbuf, spotLight->pointLight.constant);

        snprintf(prtbuf, bufsz,"sLights[%u].pointLight.linear",i);
        dengine_shader_set_float(shader, prtbuf, spotLight->pointLight.linear);

        snprintf(prtbuf, bufsz,"sLights[%u].pointLight.quadratic",i);
        dengine_shader_set_float(shader, prtbuf, spotLight->pointLight.quadratic);

        snprintf(prtbuf, bufsz,"sLights[%u].pointLight.shadow.far_shadow",i);
        dengine_shader_set_float(shader, prtbuf, spotLight->pointLight.shadow.far_shadow);

        snprintf(prtbuf, bufsz,"sLights[%u].pointLight.light.strength",i);
        dengine_shader_set_float(shader, prtbuf, spotLight->pointLight.light.strength);

        snprintf(prtbuf, bufsz,"sLights[%u].innerCutOff",i);
        dengine_shader_set_float(shader, prtbuf, iCut_rad);

        snprintf(prtbuf, bufsz,"sLights[%u].outerCutOff",i);
        dengine_shader_set_float(shader, prtbuf, oCut_rad);
    }

    //Compatibility until shaders are upgraded

    dengine_shader_set_vec3(shader, "lightPos", spotLight->pointLight.position);
    dengine_shader_set_vec3(shader, "lightDir", spotLight->direction);
    float diffStrength[3];
    memcpy(diffStrength, spotLight->pointLight.light.diffuse, sizeof(diffStrength));;

    for(int i = 0; i < 3; i++)
        diffStrength[i]*=spotLight->pointLight.light.strength;

    dengine_shader_set_vec3(shader, "diffuseCol", diffStrength);
    dengine_shader_set_float(shader, "constant", spotLight->pointLight.constant);
    dengine_shader_set_float(shader, "linear", spotLight->pointLight.linear);
    dengine_shader_set_float(shader, "quadratic", spotLight->pointLight.quadratic);
    dengine_shader_set_float(shader, "shadowfar", spotLight->pointLight.shadow.far_shadow);

    dengine_shader_set_float(shader, "oCut", oCut_rad);
    dengine_shader_set_float(shader, "iCut", iCut_rad);
}

void dengine_lighting_shadow_spotlight_draw(SpotLight* spotLight, Shader* shader, Primitive* primitive, float* modelmtx)
{
    dengine_lighting_shadow_pointlight_draw(&spotLight->pointLight, shader, primitive, modelmtx);
}

void dengine_lighting_terminate()
{
    if(lighting.pLights)
        free(lighting.pLights);

    if(lighting.sLights)
        free(lighting.sLights);
}
