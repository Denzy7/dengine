#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stddef.h> //size_t
#include <stdint.h> //uint32
typedef struct Framebuffer
{
    uint32_t framebuffer_id;
}Framebuffer;

#ifdef __cplusplus
extern "C" {
#endif

void dengine_framebuffer_gen(size_t count, Framebuffer* framebuffers);

void dengine_framebuffer_bind(uint32_t target, Framebuffer* framebuffer);

#ifdef __cplusplus
}
#endif
#endif // FRAMEBUFFER_H
