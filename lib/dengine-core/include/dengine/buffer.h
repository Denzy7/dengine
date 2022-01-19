#ifndef BUFFER_H
#define BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h> //uint32
#include <stddef.h> //size_t
typedef struct Buffer{
    uint32_t buffer_id;
    void* data;
    size_t size;
    uint32_t usage;
} Buffer;

void dengine_buffer_gen(size_t count, Buffer* buffers);

void dengine_buffer_bind(uint32_t target, Buffer* buffer);

void dengine_buffer_data(uint32_t target, Buffer* buffer);

#ifdef __cplusplus
}
#endif

#endif // BUFFER_H
