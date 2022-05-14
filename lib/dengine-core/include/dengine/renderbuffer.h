#ifndef RENDERBUFFER_H
#define RENDERBUFFER_H

#include <stdint.h> //uint32_t
#include <stddef.h> //size_t
typedef struct
{
    uint32_t renderbuffer_id;

    uint32_t format;
    uint32_t width;
    uint32_t height;
}Renderbuffer;

#ifdef __cplusplus
extern "C" {
#endif
void dengine_renderbuffer_gen(size_t count, Renderbuffer* renderbuffers);

void dengine_renderbuffer_destroy(size_t count, Renderbuffer* renderbuffers);

void dengine_renderbuffer_bind(Renderbuffer* renderbuffer);

void dengine_renderbuffer_data(Renderbuffer* renderbuffer);
#ifdef __cplusplus
}
#endif
#endif // RENDERBUFFER_H
