#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stddef.h> //size_t
#include <stdint.h> //uint32
typedef struct Framebuffer
{
    uint32_t framebuffer_id;
}Framebuffer;

void dengine_framebuffer_gen(size_t count, Framebuffer* framebuffers);

void dengine_framebuffer_bind(uint32_t target, Framebuffer* framebuffer);

#endif // FRAMEBUFFER_H
