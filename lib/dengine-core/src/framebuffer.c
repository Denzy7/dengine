#include "dengine/framebuffer.h"

#include "dengine-utils/logging.h"
#include "dengine-utils/debug.h"

#include "dengine/loadgl.h"

#include <string.h> //memset
void dengine_framebuffer_gen(size_t count, Framebuffer* framebuffers)
{
    DENGINE_DEBUG_ENTER;

    for(size_t i = 0; i < count; i++)
    {
        memset(&framebuffers[i], 0, sizeof(Framebuffer));
#ifdef DENGINE_GL_GLAD
        if(glad_glGenFramebuffers)
            glGenFramebuffers(1, &framebuffers[i].framebuffer_id);
        else if(glad_glGenFramebuffersEXT)
            glGenFramebuffersEXT(1, &framebuffers[i].framebuffer_id);
#endif
        DENGINE_CHECKGL;
    }
}

void dengine_framebuffer_bind(uint32_t target, Framebuffer* framebuffer)
{
    DENGINE_DEBUG_ENTER;

    uint32_t fb = 0;
    if(framebuffer)
        fb = framebuffer->framebuffer_id;
#ifdef DENGINE_GL_GLAD
    if(glad_glBindFramebuffer)
        glBindFramebuffer(target, fb);
    else if(glad_glBindFramebufferEXT)
        glBindFramebufferEXT(target, fb);
#endif
    DENGINE_CHECKGL;
}

void dengine_framebuffer_attach(FramebufferAttachmentType attachment, Texture* texture, Framebuffer* framebuffer)
{
    DENGINE_DEBUG_ENTER;

    //Guard against this dangerous call
    //on GL/ES 3.2+
    #ifdef DENGINE_GL_GLAD
    if(!glad_glFramebufferTexture)
    {
        dengineutils_logging_log("WARNING::FRAMEBUFFER::3D cubemap not attached coz glFramebufferTexture was not loaded");
        return;
    }

    GLenum attach = GL_COLOR_ATTACHMENT0 + framebuffer->n_color;
    if(attachment == DENGINE_FRAMEBUFFER_DEPTH)
        attach = GL_DEPTH_ATTACHMENT;
    else if (attachment == DENGINE_FRAMEBUFFER_STENCIL)
        attach = GL_STENCIL_ATTACHMENT;

    if(glad_glFramebufferTexture)
        glFramebufferTexture(GL_FRAMEBUFFER, attach, texture->texture_id, 0);

    if(!DENGINE_CHECKGL)
    {
        if(attachment == DENGINE_FRAMEBUFFER_DEPTH)
        {framebuffer->depth = *texture;}
        else if(attachment == DENGINE_FRAMEBUFFER_STENCIL)
        {framebuffer->stencil = *texture;}
        else if(attachment == DENGINE_FRAMEBUFFER_COLOR)
        {
            framebuffer->color[framebuffer->n_color] = *texture;
            framebuffer->n_color++;
        }
    }

    #endif
}

void dengine_framebuffer_attach2D(FramebufferAttachmentType attachment, Texture* texture, Framebuffer* framebuffer)
{
    DENGINE_DEBUG_ENTER;

    GLenum attach = GL_COLOR_ATTACHMENT0 + framebuffer->n_color;
    if(attachment == DENGINE_FRAMEBUFFER_DEPTH)
        attach = GL_DEPTH_ATTACHMENT;
    else if (attachment == DENGINE_FRAMEBUFFER_STENCIL)
        attach = GL_STENCIL_ATTACHMENT;

    //on GL 3.0 / ES 2.0+, or with ext GL_EXT/ARB_framebuffer_object for < 3.0
#ifdef DENGINE_GL_GLAD
    if(glad_glFramebufferTexture2D)
        glFramebufferTexture2D(GL_FRAMEBUFFER, attach, GL_TEXTURE_2D, texture->texture_id, 0);
    else if (glad_glFramebufferTexture2DEXT)
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER, attach, GL_TEXTURE_2D, texture->texture_id, 0);
    else
        dengineutils_logging_log("WARNING::Unable to find suitable glFramebufferTexture2D");
#endif

    if(!DENGINE_CHECKGL)
    {
        if(attachment == DENGINE_FRAMEBUFFER_DEPTH)
        {framebuffer->depth = *texture;}
        else if(attachment == DENGINE_FRAMEBUFFER_STENCIL)
        {framebuffer->stencil = *texture;}
        else if(attachment == DENGINE_FRAMEBUFFER_COLOR)
        {
            framebuffer->color[framebuffer->n_color] = *texture;
            framebuffer->n_color++;
        }
    }
}

void dengine_framebuffer_attachRB(FramebufferAttachmentType attachment, Renderbuffer* renderbuffer, Framebuffer* framebuffer)
{
    GLenum attach = GL_COLOR_ATTACHMENT0 + framebuffer->n_color;
    if(attachment == DENGINE_FRAMEBUFFER_DEPTH)
        attach = GL_DEPTH_ATTACHMENT;
    else if (attachment == DENGINE_FRAMEBUFFER_STENCIL)
        attach = GL_STENCIL_ATTACHMENT;
    DENGINE_DEBUG_ENTER;
#ifdef DENGINE_GL_GLAD
    if(glad_glFramebufferRenderbuffer)
    {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attach,
                                  GL_RENDERBUFFER, renderbuffer->renderbuffer_id);
    }else if(glad_glFramebufferRenderbufferEXT)
    {
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, attach,
                                  GL_RENDERBUFFER, renderbuffer->renderbuffer_id);
    }
    DENGINE_CHECKGL;
#endif
}

void dengine_framebuffer_readback(Texture* dest, Framebuffer* framebuffer)
{
    dengine_framebuffer_bind(GL_FRAMEBUFFER, framebuffer);
    glReadPixels(0, 0, dest->width, dest->height, dest->format, dest->type, dest->data);
    glFinish();
    DENGINE_CHECKGL;
    dengine_framebuffer_bind(GL_FRAMEBUFFER, NULL);
}
