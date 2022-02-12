#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include "buffer.h"
#include "vertex.h"

#include <stdint.h> //uint32
//#include <stddef.h> //size_t

typedef struct Primitive
{
    uint32_t vao;

    Buffer array;
    Buffer index;

    uint32_t index_count;

    VertexAttribute aPos;
    VertexAttribute aTexCoord;
    VertexAttribute aNormal;

    uint32_t draw_mode;
    uint32_t draw_type;
    void* offset;
} Primitive;

#ifdef __cplusplus
extern "C" {
#endif

void dengine_primitive_setup(Primitive* primitive, Shader* shader);

void dengine_primitive_gen_quad(Primitive* primitive, Shader* shader);

void dengine_primitive_gen_plane(Primitive* primitive, Shader* shader);

void dengine_primitive_gen_cube(Primitive* primitive, Shader* shader);

void dengine_primitive_gen_grid(uint16_t slice, Primitive* primitive, Shader* shader);

void dengine_primitive_gen_axis(Primitive* primitive, Shader* shader);

#ifdef __cplusplus
}
#endif

#endif // PRIMITIVE_H
