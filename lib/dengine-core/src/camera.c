#include "dengine/camera.h"

#include "dengine/loadgl.h"
#include "dengine/entrygl.h"

#include "dengine-utils/debug.h"
#include "dengine-utils/macros.h"
#include "dengine/shader.h"
#include "dengine/texture.h"

#include <cglm/cglm.h> //glm_proj
#include <string.h>    //memcpy

static float _target_zero[3] = {0.0f, 0.0f, 0.0f};
static const float _default_distance = 7.0f;
static const float _default_clearcol[4] = {1.0f, 0.5f, 0.3f, 1.0};

static const char* possible_projmat[]=
{
  "projection", "camera.projection", "proj",
};

static const char* possible_viewmat[]=
{
  "view", "camera.view"
};

static const char* possible_uviewmat[]=
{
  "uview", "camera.uview"
};

static const char* possible_campos[]=
{
  "ViewPos", "camera.position"
};

void dengine_camera_setup(Camera* camera)
{
    DENGINE_DEBUG_ENTER;

    memset(camera, 0, sizeof (Camera));
    camera->fov = 60.0f;
    camera->near = 0.01f;
    camera->far = 100.0f;
    for (int i = 0; i < 3; i++) {
        camera->position[i] = _default_distance;
    }
    memcpy(camera->clearcolor, _default_clearcol, sizeof (_default_clearcol));
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    DENGINE_CHECKGL;
    camera->render_height = viewport[3];
    camera->render_width = viewport[2];

    dengine_camera_project_perspective( (float)camera->render_width / (float)camera->render_height, camera);
    dengine_camera_lookat(NULL, camera);
    camera->clearonuse = 1;
}

void dengine_camera_project_perspective(const float aspect, Camera* camera)
{
    DENGINE_DEBUG_ENTER;

    mat4 proj;
    glm_perspective(glm_rad(camera->fov), aspect, camera->near, camera->far, proj);
    memcpy(camera->projection_mat, proj, sizeof(camera->projection_mat));
}

void dengine_camera_lookat(float* target, Camera* camera)
{
    DENGINE_DEBUG_ENTER;

    mat4 view;
    mat3 view_3x3;
    mat4 uview;
    vec3 up = {0.0f, 1.0f, 0.0f};

    float* _target = target;
    if (!_target) {
        _target = _target_zero;
    }

    glm_lookat(camera->position, _target, up, view);
    memcpy(camera->view_mat, view, sizeof(camera->view_mat));

    glm_mat4_pick3(view, view_3x3);
    glm_mat4_zero(uview);
    glm_mat4_ins3(view_3x3, uview);

    memcpy(camera->uview_mat, uview, sizeof(camera->uview_mat));
}

void dengine_camera_apply(const Shader* shader, const Camera* camera)
{
    DENGINE_DEBUG_ENTER;

    dengine_shader_use(shader);
    for (size_t i = 0; i < DENGINE_ARY_SZ(possible_projmat); i++) {
        dengine_shader_current_set_mat4(possible_projmat[i], camera->projection_mat);
    }

    for (size_t i = 0; i < DENGINE_ARY_SZ(possible_viewmat); i++) {
        dengine_shader_current_set_mat4(possible_viewmat[i], camera->view_mat);
    }

    for (size_t i = 0; i < DENGINE_ARY_SZ(possible_uviewmat); i++) {
        dengine_shader_current_set_mat4(possible_uviewmat[i], camera->uview_mat);
    }

    for (size_t i = 0; i < DENGINE_ARY_SZ(possible_campos); i++) {
        dengine_shader_current_set_vec3(possible_campos[i], camera->position);
    }
}

void dengine_camera_set_rendermode(CameraRenderMode mode, Camera* camera)
{
    DENGINE_DEBUG_ENTER;

    // get entry stuff
    Texture entry_tex;
    dengine_entrygl_texture(GL_TEXTURE_2D, &entry_tex);
    Framebuffer entry_fb;
    dengine_entrygl_framebuffer(GL_FRAMEBUFFER, &entry_fb);

    if (mode == DENGINE_CAMERA_RENDER_FOWARD) {
        Texture rgba_depth[2];
        memset(&rgba_depth,0,sizeof (rgba_depth));

        dengine_texture_gen(2, rgba_depth);
        Texture* rgba =  &rgba_depth[0];
        Texture* depth =  &rgba_depth[1];  //Use renderbuffer?

        //rgba
        rgba->format = GL_RGBA;
        rgba->internal_format = GL_RGBA;
        rgba->width =  camera->render_width;
        rgba->height = camera->render_height;
        rgba->filter_mag = GL_LINEAR;
        rgba->filter_min = GL_LINEAR;
        rgba->type = GL_UNSIGNED_BYTE;

        dengine_texture_bind(GL_TEXTURE_2D, rgba);
        dengine_texture_data(GL_TEXTURE_2D, rgba);
        dengine_texture_set_params(GL_TEXTURE_2D, rgba);

        //depth
        depth->format = GL_DEPTH_COMPONENT;
        depth->internal_format = GL_DEPTH_COMPONENT;
        depth->width = camera->render_width;
        depth->height = camera->render_height;
        depth->type = GL_UNSIGNED_SHORT;
        /*
         * GL_OES_depth_texture allows  GL_UNSIGNED_INT or GL_UNSIGNED_SHORT. SHORT has 
         * very poor precision esp. on new hardware. use highest precision 
         * where possible
         */
        if(dengine_texture_issupprorted(GL_TEXTURE_2D, GL_UNSIGNED_INT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT))
            depth->type = GL_UNSIGNED_INT;

        dengine_texture_bind(GL_TEXTURE_2D, depth);
        dengine_texture_data(GL_TEXTURE_2D, depth);
        dengine_texture_set_params(GL_TEXTURE_2D, depth);

        dengine_framebuffer_gen(1, &camera->framebuffer);
        dengine_framebuffer_bind(GL_FRAMEBUFFER, &camera->framebuffer);
        dengine_framebuffer_attach2D(DENGINE_FRAMEBUFFER_COLOR, rgba, &camera->framebuffer);
        dengine_framebuffer_attach2D(DENGINE_FRAMEBUFFER_DEPTH, depth, &camera->framebuffer);

        dengine_texture_bind(GL_TEXTURE_2D, &entry_tex);
        dengine_framebuffer_bind(GL_FRAMEBUFFER, &entry_fb);
    }
}

void dengine_camera_use(const Camera* camera)
{
    DENGINE_DEBUG_ENTER;

    if (camera && camera->framebuffer.color->texture_id == 0)
        return;

    if (camera) {
        float r,g,b,a;
        dengine_framebuffer_get_clearcolor(&r, &g, &b, &a);
        glClearColor(
                camera->clearcolor[0],
                camera->clearcolor[1],
                camera->clearcolor[2],
                camera->clearcolor[3]);
        DENGINE_CHECKGL;

        dengine_framebuffer_bind(GL_FRAMEBUFFER, &camera->framebuffer);
        if(camera->clearonuse){
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            DENGINE_CHECKGL;
        }
        glClearColor(r, g, b, a);
        DENGINE_CHECKGL;
    }else
    {
        dengine_framebuffer_bind(GL_FRAMEBUFFER, NULL);
    }
}

void dengine_camera_resize(Camera* camera, int width, int height)
{
    DENGINE_DEBUG_ENTER;

    Texture entry_tex;
    dengine_entrygl_texture(GL_TEXTURE_2D, &entry_tex);

    for(uint32_t i = 0; i < camera->framebuffer.n_color; i++)
    {
        Texture* col = &camera->framebuffer.color[i];
        if(col->texture_id)
        {
            dengine_texture_bind(GL_TEXTURE_2D, col);
            col->width = width;
            col->height = height;
            dengine_texture_data(GL_TEXTURE_2D, col);
        }
    }

    Texture* depth = &camera->framebuffer.depth;
    if(depth->texture_id)
    {
        dengine_texture_bind(GL_TEXTURE_2D, depth);
        depth->width = width;
        depth->height = height;
        dengine_texture_data(GL_TEXTURE_2D, depth);
    }

    camera->render_width = width;
    camera->render_height = height;

    dengine_texture_bind(GL_TEXTURE_2D, &entry_tex);
}

void dengine_camera_world2screen(const Camera* camera, const float* world, float* screen)
{
    DENGINE_DEBUG_ENTER;

    vec4 clip_v, clip_p;
    mat4 proj, view;
    vec3 ndc;
    vec2 sz = {camera->render_width, camera->render_height};
    vec4 vp;

    memcpy(&proj[0][0], camera->projection_mat, sizeof(camera->projection_mat));
    memcpy(&view[0][0], camera->view_mat, sizeof(camera->view_mat));
    glm_vec4_one(vp);
    memcpy(vp, world, sizeof(vec3));

    glm_mat4_mulv(view, vp, clip_v);
    glm_mat4_mulv(proj, clip_v, clip_p);
    glm_vec3_divs(clip_p, clip_p[3], ndc);
    glm_vec2_adds(ndc, 1.0, ndc);
    glm_vec2_divs(ndc, 2.0, ndc);
    glm_vec2_mul(ndc, sz, ndc);
    memcpy(screen, ndc, sizeof(vec2));
}
