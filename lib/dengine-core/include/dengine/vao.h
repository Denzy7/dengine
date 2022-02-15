#ifndef VAO_H
#define VAO_H

#include <stdint.h> //uint32
#include <stddef.h> //size_t
typedef struct VAO
{
    uint32_t vao;
}VAO;

#ifdef __cplusplus
extern "C" {
#endif

void dengine_vao_gen(size_t count, VAO* vaos);

void dengine_vao_bind(VAO* vao);

#ifdef __cplusplus
}
#endif

#endif // VAO_H
