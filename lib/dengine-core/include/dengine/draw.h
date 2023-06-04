/*! \file draw.h
 *  Execute draw commands
 */

#ifndef DRAW_H
#define DRAW_H

#include "dengine/primitive.h" //draw primitive
#include "dengine/shader.h" //use_program
#include <stdint.h> //uint32_t

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Restore previous objects(shader, buffers, etc.) used before the draw call
 *
 * Useful for embedding (into GTKGLArea or something else) to avoid losing previous objects.
 * Has a slight overhead so disable if not embedding (disabled by default)
 * \param entrygl
 */
void dengine_draw_enable_entrygl(int state);

/*!
 * \brief Execute glDrawElements on currently bound framebuffer
 * \param primitive Primitive to draw
 * \param shader Shader to use
 */
void dengine_draw_primitive(const Primitive* primitive, const Shader* shader);

#ifdef __cplusplus
}
#endif
#endif // DRAW_H
