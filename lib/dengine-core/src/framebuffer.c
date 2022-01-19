#include "framebuffer.h"

#include "loadgl.h"

void dengine_framebuffer_gen(size_t count, Framebuffer* framebuffers)
{
    for(size_t i = 0; i < count; i++)
        glGenFramebuffers(1, &framebuffers[i].framebuffer_id); DENGINE_CHECKGL
}

void dengine_framebuffer_bind(uint32_t target, Framebuffer* framebuffer)
{
    glBindFramebuffer(target, framebuffer->framebuffer_id); DENGINE_CHECKGL
}
