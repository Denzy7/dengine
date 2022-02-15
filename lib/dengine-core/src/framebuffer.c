#include "framebuffer.h"

#include "loadgl.h"

#include <string.h> //memset
void dengine_framebuffer_gen(size_t count, Framebuffer* framebuffers)
{
    for(size_t i = 0; i < count; i++)
    {
        memset(&framebuffers[i], 0, sizeof(Framebuffer));
        glGenFramebuffers(1, &framebuffers[i].framebuffer_id); DENGINE_CHECKGL;
    }
}

void dengine_framebuffer_bind(uint32_t target, Framebuffer* framebuffer)
{
    if(framebuffer)
        glBindFramebuffer(target, framebuffer->framebuffer_id);
    else
        glBindFramebuffer(target, 0);

    DENGINE_CHECKGL;
}

void dengine_framebuffer_attach(FramebufferAttachmentType attachment, Texture* texture, Framebuffer* framebuffer)
{
    //Guard against this dangerous call
    #ifdef DENGINE_GL_GLAD
    if(!glad_glFramebufferTexture)
        return;
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
    if(attachment == DENGINE_FRAMEBUFFER_COLOR){
        //on GL 3.0 / ES 2.0+
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment + framebuffer->n_color, GL_TEXTURE_2D, texture->texture_id, 0);
        if(!DENGINE_CHECKGL){
            framebuffer->color[framebuffer->n_color] = *texture;
            framebuffer->n_color++;
        }
    }
    else{
        //on GL 3.0 / ES 2.0+
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture->texture_id, 0);
        if(!DENGINE_CHECKGL)
        {
            if(attachment == DENGINE_FRAMEBUFFER_DEPTH)
                framebuffer->depth = *texture;
            else if(attachment == DENGINE_FRAMEBUFFER_STENCIL)
                framebuffer->stencil = *texture;
        }
    }
}
