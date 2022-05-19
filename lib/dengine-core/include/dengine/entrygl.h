#ifndef ENTRYGL_H
#define ENTRYGL_H

#include "dengine/texture.h"
#include "dengine/framebuffer.h"
#include "dengine/buffer.h"
#include "dengine/vao.h"
#include "dengine/shader.h"

#ifdef __cplusplus
extern "C" {
#endif

const Texture* dengine_entrygl_texture(uint32_t target);

const int dengine_entrygl_texture_active();

const Framebuffer* dengine_entrygl_framebuffer(uint32_t target);

const Buffer* dengine_entrygl_buffer(uint32_t target);

const VAO* dengine_entrygl_vao();

const Shader* dengine_entrygl_shader();

#ifdef __cplusplus
}
#endif

#endif // ENTRYGL_H
