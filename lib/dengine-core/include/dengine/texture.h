#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdint.h> //uint32
#include <stddef.h> //size_t

enum
{
    DENGINE_TEXTURE_INTERFACE_8_BIT = 1,
    DENGINE_TEXTURE_INTERFACE_16_BIT,
    DENGINE_TEXTURE_INTERFACE_FLOAT
};

typedef struct Texture
{
    uint32_t texture_id;
    int width, height, channels;
    void* data;
    uint32_t interface;
    uint32_t internal_format, format;
    uint32_t type;
    uint32_t filter_mag, filter_min;
    uint32_t wrap;
    int mipmap;

    char* name;
} Texture;

#ifdef __cplusplus
extern "C" {
#endif

void dengine_texture_gen(size_t count, Texture* textures);

void dengine_texture_bind(uint32_t target, Texture* texture);

void dengine_texture_data(uint32_t target, Texture* texture);

int dengine_texture_load_mem(void* mem, size_t size, int flip, Texture* texture);

int dengine_texture_load_file(const char* file, int flip, Texture* texture);

void dengine_texture_set_params(uint32_t target, Texture* texture);

void dengine_texture_free_data(Texture* texture);

void dengine_texture_destroy(size_t count, Texture* textures);

#ifdef __cplusplus
}
#endif

#endif // TEXTURE_H
