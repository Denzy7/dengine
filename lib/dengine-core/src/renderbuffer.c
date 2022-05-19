#include "dengine/renderbuffer.h"
#include "dengine/loadgl.h" //GL

void dengine_renderbuffer_gen(const size_t count, Renderbuffer* renderbuffers)
{
    for(size_t i = 0; i < count; i++)
    {
        if(glad_glGenRenderbuffers)
            glGenRenderbuffers(1, &renderbuffers->renderbuffer_id);
        else if(glad_glGenRenderbuffersEXT)
            glGenRenderbuffersEXT(1, &renderbuffers->renderbuffer_id);

        DENGINE_CHECKGL;
    }
}

void dengine_renderbuffer_destroy(const size_t count, Renderbuffer* renderbuffers)
{
    for(size_t i = 0; i < count; i++)
    {
        if(glad_glDeleteRenderbuffers)
            glDeleteRenderbuffers(1, &renderbuffers->renderbuffer_id);
        else if(glad_glDeleteRenderbuffersEXT)
            glDeleteRenderbuffersEXT(1, &renderbuffers->renderbuffer_id);

        DENGINE_CHECKGL;
    }
}

void dengine_renderbuffer_bind(const Renderbuffer* renderbuffer)
{
    uint32_t bind = 0;
    if(renderbuffer)
        bind = renderbuffer->renderbuffer_id;
    if(glad_glBindRenderbuffer)
        glBindRenderbuffer(GL_RENDERBUFFER, bind);
    else if(glad_glBindRenderbufferEXT)
        glBindRenderbufferEXT(GL_RENDERBUFFER, bind);

    DENGINE_CHECKGL;
}

void dengine_renderbuffer_data(const Renderbuffer* renderbuffer)
{
    if(glad_glRenderbufferStorage)
    {
        glRenderbufferStorage(GL_RENDERBUFFER, renderbuffer->format,
                              renderbuffer->width, renderbuffer->height);
    }else if(glad_glRenderbufferStorageEXT)
    {
        glRenderbufferStorageEXT(GL_RENDERBUFFER, renderbuffer->format,
                              renderbuffer->width, renderbuffer->height);
    }

    DENGINE_CHECKGL;
}
