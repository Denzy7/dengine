#ifndef VERTEX_H
#define VERTEX_H

#include <stddef.h> //size_t
#include <stdint.h> //uint32

#include "shader.h" //Shader

typedef struct VertexAttribute
{
    uint32_t index;
    char* location;
    int size;
    uint32_t type;
    uint32_t normalize;
    size_t stride;
    void* ptr;
}VertexAttribute;

typedef struct VertexArray
{
    uint32_t vao_id;
    uint32_t* attributes;
    size_t attributes_len;
}VertexArray;

#ifdef __cplusplus
extern "C" {
#endif

void dengine_vertex_attribute_setup(VertexAttribute* attribute);

void dengine_vertex_attribute_enable(VertexAttribute* attribute);

void dengine_vertex_attribute_indexfromshader(Shader* shader, VertexAttribute* attribute, const char* attribute_name);

#ifdef __cplusplus
}
#endif

#endif // VERTEXARRAY_H
