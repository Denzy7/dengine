#include "framebuffer.h"

#include "dengine-utils/logging.h"
#include "dengine-utils/debug.h"

#include "loadgl.h"

#include <string.h> //memset
void dengine_framebuffer_gen(size_t count, Framebuffer* framebuffers)
{
    DENGINE_DEBUG_ENTER;

    for(size_t i = 0; i < count; i++)
    {
        memset(&framebuffers[i], 0, sizeof(Framebuffer));
        if(glad_glGenFramebuffers)
            glGenFramebuffers(1, &framebuffers[i].framebuffer_id);
        else if(glad_glGenFramebuffersEXT)
            glGenFramebuffersEXT(1, &framebuffers[i].framebuffer_id);

        DENGINE_CHECKGL;
    }
}

void dengine_framebuffer_bind(uint32_t target, Framebuffer* framebuffer)
{
    DENGINE_DEBUG_ENTER;

    uint32_t fb = 0;
    if(framebuffer)
        fb = framebuffer->framebuffer_id;

    if(glad_glBindFramebuffer)
        glBindFramebuffer(target, fb);
    else if(glad_glBindFramebufferEXT)
        glBindFramebufferEXT(target, fb);

    DENGINE_CHECKGL;
}

void dengine_framebuffer_attach(FramebufferAttachmentType attachment, Texture* texture, Framebuffer* framebuffer)
{
    DENGINE_DEBUG_ENTER;

    //Guard against this dangerous call
    #ifdef DENGINE_GL_GLAD
    if(!glad_glFramebufferTexture)
    {
        dengineutils_logging_log("WARNING::FRAMEBUFFER::3D cubemap not attached coz glFramebufferTexture was not loaded");
        return;
    }
    #endif

    if(attachment == DENGINE_FRAMEBUFFER_COLOR){
        //on GL/ES 3.2+
        glFramebufferTexture(GL_FRAMEBUFFER, attachment + framebuffer->n_color, texture->texture_id, 0);
        if(!DENGINE_CHECKGL)
        {
            framebuffer->color[framebuffer->n_color] = *texture;
            framebuffer->n_color++;
        }
    }
    else{
        glFramebufferTexture(GL_FRAMEBUFFER, attachment, texture->texture_id, 0);
        if(!DENGINE_CHECKGL)
        {
            if(attachment == DENGINE_FRAMEBUFFER_DEPTH)
                framebuffer->depth = *texture;
            else if(attachment == DENGINE_FRAMEBUFFER_STENCIL)
                framebuffer->stencil = *texture;
        }
    }
}

void dengine_framebuffer_attach2D(FramebufferAttachmentType attachment, Texture* texture, Framebuffer* framebuffer)
{
    DENGINE_DEBUG_ENTER;

    //on GL 3.0 / ES 2.0+, or with ext GL_EXT/ARB_framebuffer_object for < 3.0
    if(attachment == DENGINE_FRAMEBUFFER_COLOR){
        if(glad_glFramebufferTexture2D)
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachment + framebuffer->n_color, GL_TEXTURE_2D, texture->texture_id, 0);
        else if(glad_glFramebufferTexture2DEXT)
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER, attachment + framebuffer->n_color, GL_TEXTURE_2D, texture->texture_id, 0);

        if(!DENGINE_CHECKGL){
            framebuffer->color[framebuffer->n_color] = *texture;
            framebuffer->n_color++;
        }
    }
    else{
        if(glad_glFramebufferTexture2D)
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture->texture_id, 0);
        else if(glad_glFramebufferTexture2DEXT)
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture->texture_id, 0);

        if(!DENGINE_CHECKGL)
        {
            if(attachment == DENGINE_FRAMEBUFFER_DEPTH)
                framebuffer->depth = *texture;
            else if(attachment == DENGINE_FRAMEBUFFER_STENCIL)
                framebuffer->stencil = *texture;
        }
    }
}
