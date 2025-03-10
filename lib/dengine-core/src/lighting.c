#include "dengine/lighting.h"

#include "dengine/loadgl.h" //gl
#include "dengine/draw.h" //draw
//#include "dengine/window.h" //get w,h
#include "dengine/entrygl.h" //entryfb

#include "dengine-utils/logging.h"
#include "dengine-utils/macros.h" //arr_sz
#include "dengine-utils/debug.h"
#include "dengine/shader.h"

#include <string.h> //memset

#include <stdlib.h> //malloc
#include <assert.h> //assert =

#include <cglm/cglm.h> //mat, proj
#include <math.h> /* cos */        

Lighting lighting;

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

static const float default_ambient[]=
{
    .1f,.1f,.1f,1.0f
};

static const float default_specular[]=
{
    .75f,.75f,.75f,1.0f
};

static const float default_diffuse[]=
{
    .75f,.75f,.75f,1.0f
};

void _dengine_lighting_shadowop_setup(uint32_t shadowmap_target, ShadowOp* shadowop);

void _dengine_lighting_shadowop_setup(uint32_t shadowmap_target, ShadowOp* shadowop)
{
    Texture depth;
    memset(&depth, 0, sizeof(Texture));
    depth.type = GL_UNSIGNED_SHORT;
    
    if(!dengine_texture_issupprorted(shadowmap_target, GL_UNSIGNED_SHORT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT))
    {
        dengineutils_logging_log("WARNING::GPU doesn't support depth texture as target: [ %s ] shadowmap not generated!",
                dengine_texture_target2str(shadowmap_target));
        return;
    }

    if(dengine_texture_issupprorted(shadowmap_target, GL_UNSIGNED_INT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT))
    {
        depth.type = GL_UNSIGNED_INT;
    }else {
        dengineutils_logging_log("WARNING::GPU doesn't support 32bit depth. Falling back to 16bit");
    }

    shadowop->invisiblemesh = 0;

    Framebuffer entry_fb;
    dengine_entrygl_framebuffer(GL_FRAMEBUFFER, &entry_fb);
    Texture entry_tex;
    dengine_entrygl_texture(shadowmap_target, &entry_tex );

    depth.height = shadowop->shadow_map_size;
    depth.width = shadowop->shadow_map_size;
    depth.format = GL_DEPTH_COMPONENT;
    depth.internal_format = GL_DEPTH_COMPONENT;
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

    if(glad_glReadBuffer)
    {glReadBuffer(GL_NONE); DENGINE_CHECKGL;}

    if(glad_glDrawBuffers)
    {
        GLenum none[] = {GL_NONE};
        glDrawBuffers(1, none); DENGINE_CHECKGL;
    }

    dengine_texture_bind(shadowmap_target, &entry_tex);
    dengine_framebuffer_bind(GL_FRAMEBUFFER, &entry_fb);

    shadowop->near_shadow = 0.01f;
    shadowop->far_shadow = 25.0f;
    shadowop->enable = 1;
    shadowop->max_bias = 0.001;
}

int dengine_lighting_init(const uint32_t n_PL, const uint32_t n_SL)
{
    DENGINE_DEBUG_ENTER;

    return 0;
}

Lighting* dengine_lighting_get()
{
    DENGINE_DEBUG_ENTER;

    return &lighting;
}

void dengine_lighting_shadowop_clear(const ShadowOp* shadowop)
{
    DENGINE_DEBUG_ENTER;

    if (shadowop->shadow_map.depth.texture_id == 0)
        return;

    //we might not have entered with fb 0, save binding for later
    Framebuffer entry_fb;
    dengine_entrygl_framebuffer(GL_FRAMEBUFFER, &entry_fb);

    dengine_framebuffer_bind(GL_FRAMEBUFFER, &shadowop->shadow_map);
    glClear(GL_DEPTH_BUFFER_BIT); DENGINE_CHECKGL;
    DENGINE_CHECKGL;

    //bind entry fb
    dengine_framebuffer_bind(GL_FRAMEBUFFER, &entry_fb);
}

void dengine_lighting_shadowop_resize(uint32_t shadowmap_target, ShadowOp* shadowop, int size)
{
    DENGINE_DEBUG_ENTER;

    Texture entry_tex;
    dengine_entrygl_texture(shadowmap_target, &entry_tex);

    Texture* depth = &shadowop->shadow_map.depth;
    if(depth->texture_id)
    {
        dengine_texture_bind(shadowmap_target, depth);
        depth->width = size;
        depth->height = size;

        if(shadowmap_target == GL_TEXTURE_CUBE_MAP)
        {
            for(int i = GL_TEXTURE_CUBE_MAP_POSITIVE_X; i < 6; i++)
            {
                dengine_texture_data(i, depth);
            }
        }else
        {
            dengine_texture_data(shadowmap_target, depth);
        }
    }

    shadowop->shadow_map_size = size;
    dengine_texture_bind(shadowmap_target, &entry_tex);
}

void dengine_lighting_setup_dirlight(DirLight* dirLight)
{
    DENGINE_DEBUG_ENTER;

    if(dirLight->shadow.enable)
    {
        _dengine_lighting_shadowop_setup(GL_TEXTURE_2D, &dirLight->shadow);

        memset(dirLight->shadow_projview, 0, sizeof(dirLight->shadow_projview));
    }
    static const float default_dirlight_dir[3] =
    {
      -4.0f, 10.0f, -6.0f
    };

    memcpy(dirLight->direction, default_dirlight_dir, sizeof (default_dirlight_dir));
    memcpy(dirLight->light.ambient, default_ambient, sizeof (default_ambient));
    memcpy(dirLight->light.diffuse, default_diffuse, sizeof (default_diffuse));
    memcpy(dirLight->light.specular, default_specular, sizeof (default_specular));

    dirLight->shadow.pcf = 0;
    dirLight->shadow.pcf_samples = 4;
    dirLight->shadow_ortho = 10.0f;

    dirLight->light.strength = 0.55;
}

void dengine_lighting_shadow_dirlight_draw(DirLight* dirLight, const Shader* shader, const Primitive* primitive, const float* modelmtx)
{
    DENGINE_DEBUG_ENTER;

    //Guard for no shadow
    if(!dirLight->shadow.enable || !dirLight->shadow.shadow_map.depth.texture_id) {
        return;
    }

    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    DENGINE_CHECKGL;
    int h = viewport[3], w = viewport[2];

    //we might not have entered with fb 0, save binding for later
    Framebuffer entryfb;
    dengine_entrygl_framebuffer(GL_FRAMEBUFFER, &entryfb);

    dengine_framebuffer_bind(GL_FRAMEBUFFER, &dirLight->shadow.shadow_map);

    const float ortho_size = dirLight->shadow_ortho;
    mat4 proj, view, pv;
    vec3* dir = (vec3*)&dirLight->direction;
    vec3 normdir;

    glm_ortho(-ortho_size, ortho_size, -ortho_size, ortho_size, dirLight->shadow.near_shadow, dirLight->shadow.far_shadow, proj);
    glm_vec3_normalize_to(*dir, normdir);

    glm_lookat(*dir, normdir, GLM_YUP, view);
    glm_mat4_mul(proj, view, pv);

    dengine_shader_set_mat4(shader, "projview", pv[0]);
    dengine_shader_set_mat4(shader, "model", modelmtx);
    memcpy(dirLight->shadow_projview, pv, sizeof(dirLight->shadow_projview));

    //printf("%d %u\n", dirLight->shadow.shadow_map_size, dirLight->shadow.shadow_map.depth.texture_id);
    glViewport(0, 0, dirLight->shadow.shadow_map_size, dirLight->shadow.shadow_map_size);
    DENGINE_CHECKGL;

    dengine_draw_primitive(primitive, shader);

    glViewport(0, 0, w, h);
    DENGINE_CHECKGL;

    //bind entry fb
    dengine_framebuffer_bind(GL_FRAMEBUFFER, &entryfb);
}

void dengine_lighting_apply_dirlight(const DirLight* dirLight, const Shader* shader)
{
    DENGINE_DEBUG_ENTER;

    static const char* possible_pos[]=
    {
        "lightDir","dLight.direction"
    };

    dengine_shader_use(shader);

    for (size_t i = 0; i < DENGINE_ARY_SZ(possible_pos); i++) {
        dengine_shader_current_set_vec3(possible_pos[i], dirLight->direction);
    }

    dengine_shader_current_set_vec3("dLight.light.ambient", dirLight->light.ambient);
    dengine_shader_current_set_vec3("dLight.light.diffuse", dirLight->light.diffuse);
    dengine_shader_current_set_vec3("dLight.light.specular", dirLight->light.specular);
    dengine_shader_current_set_float("dLight.light.strength", dirLight->light.strength);
    dengine_shader_current_set_float("dLight.shadow.max_bias", dirLight->shadow.max_bias);
    dengine_shader_current_set_int("dLight.shadow.enable", dirLight->shadow.enable);
    dengine_shader_current_set_int("dLight.shadow.pcf", dirLight->shadow.pcf);
    dengine_shader_current_set_int("dLight.shadow.pcf_samples", dirLight->shadow.pcf_samples);
    dengine_shader_current_set_int("dLight.shadow.shadow_map_size", dirLight->shadow.shadow_map_size);
    dengine_shader_current_set_mat4("dL_shadow_projview", dirLight->shadow_projview);

}

void dengine_lighting_setup_pointlight(PointLight* pointLight)
{
    DENGINE_DEBUG_ENTER;

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

void dengine_lighting_apply_pointlight(const PointLight* pointLight, const Shader* shader)
{
    DENGINE_DEBUG_ENTER;

    dengine_shader_use(shader);

    char prtbuf[1024];
    size_t bufsz = sizeof (prtbuf);
    for (uint32_t i = 0; i < 1; i++) {
        snprintf(prtbuf, bufsz,"pLights[%u].position",i);
        dengine_shader_current_set_vec3(prtbuf, pointLight->position);

        snprintf(prtbuf, bufsz,"pLights[%u].light.ambient",i);
        dengine_shader_current_set_vec3(prtbuf, pointLight->light.ambient);

        snprintf(prtbuf, bufsz,"pLights[%u].light.diffuse",i);
        dengine_shader_current_set_vec3(prtbuf, pointLight->light.diffuse);

        snprintf(prtbuf, bufsz,"pLights[%u].light.specular",i);
        dengine_shader_current_set_vec3(prtbuf, pointLight->light.specular);

        snprintf(prtbuf, bufsz,"pLights[%u].constant",i);
        dengine_shader_current_set_float(prtbuf, pointLight->constant);

        snprintf(prtbuf, bufsz,"pLights[%u].linear",i);
        dengine_shader_current_set_float(prtbuf, pointLight->linear);

        snprintf(prtbuf, bufsz,"pLights[%u].quadratic",i);
        dengine_shader_current_set_float(prtbuf, pointLight->quadratic);

        snprintf(prtbuf, bufsz,"pLights[%u].shadow.far_shadow",i);
        dengine_shader_current_set_float(prtbuf, pointLight->shadow.far_shadow);

        snprintf(prtbuf, bufsz,"pLights[%u].light.strength",i);
        dengine_shader_current_set_float(prtbuf, pointLight->light.strength);
    }

    //Compatibility until tests are upgraded
    float diffStrength[3];
    memcpy(diffStrength, pointLight->light.diffuse, sizeof(diffStrength));;

    for(int j = 0; j < 3; j++)
        diffStrength[j]*=pointLight->light.strength;

    dengine_shader_current_set_vec3("lightPos", pointLight->position);
    dengine_shader_current_set_vec3("diffuseCol", diffStrength);
    dengine_shader_current_set_float("constant", pointLight->constant);
    dengine_shader_current_set_float("linear", pointLight->linear);
    dengine_shader_current_set_float("quadratic", pointLight->quadratic);
    dengine_shader_current_set_float("shadowfar", pointLight->shadow.far_shadow);
}

void dengine_lighting_shadow_pointlight_draw(PointLight* pointLight, const Shader* shader, const Primitive* primitive, const float* modelmtx)
{
    DENGINE_DEBUG_ENTER;

    //Guard for no shadow
    if (!pointLight->shadow.shadow_map.depth.texture_id|| !pointLight->shadow.enable)
        return;

    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    DENGINE_CHECKGL;
    int h = viewport[3], w = viewport[2];

    Framebuffer entry_fb;
    dengine_entrygl_framebuffer(GL_FRAMEBUFFER, &entry_fb);

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
    DENGINE_CHECKGL;

    dengine_draw_primitive(primitive, shader);

    glViewport(0, 0, w, h);
    DENGINE_CHECKGL;

    //bind entry fb
    dengine_framebuffer_bind(GL_FRAMEBUFFER, &entry_fb);
}

void dengine_lighting_setup_spotlight(SpotLight* spotLight)
{
    DENGINE_DEBUG_ENTER;

    dengine_lighting_setup_pointlight(&spotLight->pointLight);

    spotLight->innerCutOff = 35.0f;
    spotLight->outerCutOff = 45.0f;

    spotLight->pointLight.light.strength = 1.f;

    float diff[3] = {1.0f, 1.0f, 1.0f};
    memcpy(spotLight->pointLight.light.diffuse, diff, sizeof(diff));

    spotLight->pointLight.shadow.far_shadow = 30.0f;
    spotLight->pointLight.shadow.near_shadow = 0.1f;
}

void dengine_lighting_apply_spotlight(const SpotLight* spotLight, const Shader* shader)
{
    DENGINE_DEBUG_ENTER;

    float oCut_rad = cos(glm_rad(glm_clamp(spotLight->outerCutOff, 5.0f, 120.0f)));
    float iCut_rad = cos(glm_rad(glm_clamp(spotLight->innerCutOff, 1.0f, spotLight->outerCutOff - 1.0f)));

    char prtbuf[1024];
    size_t bufsz = sizeof (prtbuf);
    dengine_shader_use(shader);
    for (uint32_t i = 0; i < 1; i++) {
        snprintf(prtbuf, bufsz,"sLights[%u].pointLight.position",i);
        dengine_shader_current_set_vec3(prtbuf, spotLight->pointLight.position);

        snprintf(prtbuf, bufsz,"sLights[%u].pointLight.light.ambient",i);
        dengine_shader_current_set_vec3(prtbuf, spotLight->pointLight.light.ambient);

        snprintf(prtbuf, bufsz,"sLights[%u].pointLight.light.diffuse",i);
        dengine_shader_current_set_vec3(prtbuf, spotLight->pointLight.light.diffuse);

        snprintf(prtbuf, bufsz,"sLights[%u].pointLight.light.specular",i);
        dengine_shader_current_set_vec3(prtbuf, spotLight->pointLight.light.specular);

        snprintf(prtbuf, bufsz,"sLights[%u].direction",i);
        dengine_shader_current_set_vec3(prtbuf, spotLight->direction);

        snprintf(prtbuf, bufsz,"sLights[%u].pointLight.constant",i);
        dengine_shader_current_set_float(prtbuf, spotLight->pointLight.constant);

        snprintf(prtbuf, bufsz,"sLights[%u].pointLight.linear",i);
        dengine_shader_current_set_float(prtbuf, spotLight->pointLight.linear);

        snprintf(prtbuf, bufsz,"sLights[%u].pointLight.quadratic",i);
        dengine_shader_current_set_float(prtbuf, spotLight->pointLight.quadratic);

        snprintf(prtbuf, bufsz,"sLights[%u].pointLight.shadow.far_shadow",i);
        dengine_shader_current_set_float(prtbuf, spotLight->pointLight.shadow.far_shadow);

        snprintf(prtbuf, bufsz,"sLights[%u].pointLight.light.strength",i);
        dengine_shader_current_set_float(prtbuf, spotLight->pointLight.light.strength);

        snprintf(prtbuf, bufsz,"sLights[%u].innerCutOff",i);
        dengine_shader_current_set_float(prtbuf, iCut_rad);

        snprintf(prtbuf, bufsz,"sLights[%u].outerCutOff",i);
        dengine_shader_current_set_float(prtbuf, oCut_rad);
    }

    //Compatibility until shaders are upgraded

    dengine_shader_current_set_vec3("lightPos", spotLight->pointLight.position);
    dengine_shader_current_set_vec3("lightDir", spotLight->direction);
    float diffStrength[3];
    memcpy(diffStrength, spotLight->pointLight.light.diffuse, sizeof(diffStrength));;

    for(int i = 0; i < 3; i++)
        diffStrength[i]*=spotLight->pointLight.light.strength;

    dengine_shader_current_set_vec3("diffuseCol", diffStrength);
    dengine_shader_current_set_float("constant", spotLight->pointLight.constant);
    dengine_shader_current_set_float("linear", spotLight->pointLight.linear);
    dengine_shader_current_set_float("quadratic", spotLight->pointLight.quadratic);
    dengine_shader_current_set_float("shadowfar", spotLight->pointLight.shadow.far_shadow);

    dengine_shader_current_set_float("oCut", oCut_rad);
    dengine_shader_current_set_float("iCut", iCut_rad);
}

void dengine_lighting_shadow_spotlight_draw(SpotLight* spotLight, const Shader* shader, const Primitive* primitive, const float* modelmtx)
{
    DENGINE_DEBUG_ENTER;

    dengine_lighting_shadow_pointlight_draw(&spotLight->pointLight, shader, primitive, modelmtx);
}

void dengine_lighting_terminate()
{
    DENGINE_DEBUG_ENTER;
}

void dengine_lighting_light_setup(const LightType type, Light* light)
{
    DENGINE_DEBUG_ENTER;

    if(type == DENGINE_LIGHT_DIR)
        dengine_lighting_setup_dirlight(light);
    else if(type == DENGINE_LIGHT_POINT)
        dengine_lighting_setup_pointlight(light);
    else if(type == DENGINE_LIGHT_SPOT)
        dengine_lighting_setup_spotlight(light);
}

void dengine_lighting_light_apply(const LightType type, const Light* light, const Shader* shader)
{
    DENGINE_DEBUG_ENTER;

    if(type == DENGINE_LIGHT_DIR)
        dengine_lighting_apply_dirlight(light, shader);
    else if(type == DENGINE_LIGHT_POINT)
        dengine_lighting_apply_pointlight(light, shader);
    else if(type == DENGINE_LIGHT_SPOT)
        dengine_lighting_apply_spotlight(light, shader);
}


LightOp* dengine_lighting_light_get_lightop(LightType type, Light* light)
{
    if(type == DENGINE_LIGHT_DIR)
        return &((DirLight*)light)->light;
    else if (type == DENGINE_LIGHT_POINT) 
        return &((PointLight*)light)->light;
    else if (type == DENGINE_LIGHT_SPOT) 
        return &((SpotLight*)light)->pointLight.light;
    else
        return NULL;
}

ShadowOp* dengine_lighting_light_get_shadowop(LightType type, Light* light)
{
    if(type == DENGINE_LIGHT_DIR)
        return &((DirLight*)light)->shadow;
    else if (type == DENGINE_LIGHT_POINT) 
        return &((PointLight*)light)->shadow;
    else if (type == DENGINE_LIGHT_SPOT) 
        return &((SpotLight*)light)->pointLight.shadow;
    else
        return NULL;
}

void dengine_lighting_light_shadow_draw(LightType type, Light* light, const Shader* shader, const Primitive* primitive, const float* modelmtx)
{
    DENGINE_DEBUG_ENTER;

    if(type == DENGINE_LIGHT_DIR)
        dengine_lighting_shadow_dirlight_draw(light, shader, primitive, modelmtx);
    else if(type == DENGINE_LIGHT_POINT)
        dengine_lighting_shadow_pointlight_draw(light, shader, primitive, modelmtx);
    else if(type == DENGINE_LIGHT_SPOT)
        dengine_lighting_shadow_spotlight_draw(light, shader, primitive, modelmtx);
}
