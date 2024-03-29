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

/*!
 * \brief Restore previous objects(shader, buffers, etc.) used before the draw call
 *
 * Useful for embedding (into GTKGLArea or something else) to avoid losing previous objects.
 * Has a slight overhead so disable if not embedding (disabled by default)
 * \param state
 */
void dengine_entrygl_set_enabled(int state);

int dengine_entrygl_get_enabled();

void dengine_entrygl_texture(const uint32_t target, Texture* texture);

void dengine_entrygl_texture_active(int* active);

void dengine_entrygl_framebuffer(const uint32_t target, Framebuffer* framebuffer);

void dengine_entrygl_buffer(const uint32_t target, Buffer* buffer);

void dengine_entrygl_vao(VAO* vao);

void dengine_entrygl_shader(Shader* shader);

#ifdef __cplusplus
}
#endif

#endif // ENTRYGL_H
