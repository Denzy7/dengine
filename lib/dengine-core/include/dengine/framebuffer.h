#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stddef.h> //size_t
#include <stdint.h> //uint32

#include "dengine/texture.h"
#include "dengine/loadgl.h"
typedef enum FramebufferAttachmentType
{
    DENGINE_FRAMEBUFFER_COLOR = GL_COLOR_ATTACHMENT0,
    DENGINE_FRAMEBUFFER_DEPTH = GL_DEPTH_ATTACHMENT,
    DENGINE_FRAMEBUFFER_STENCIL = GL_STENCIL_ATTACHMENT
}FramebufferAttachmentType;

typedef struct Framebuffer
{
    uint32_t framebuffer_id;

    Texture color[8];
    uint32_t n_color;

    Texture depth;
    Texture stencil;
}Framebuffer;

#ifdef __cplusplus
extern "C" {
#endif

void dengine_framebuffer_gen(size_t count, Framebuffer* framebuffers);

void dengine_framebuffer_bind(uint32_t target, Framebuffer* framebuffer);

void dengine_framebuffer_attach(FramebufferAttachmentType attachment, Texture* texture, Framebuffer* framebuffer);

void dengine_framebuffer_attach2D(FramebufferAttachmentType attachment, Texture* texture, Framebuffer* framebuffer);
#ifdef __cplusplus
}
#endif
#endif // FRAMEBUFFER_H
