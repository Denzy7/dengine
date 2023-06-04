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
 * \brief Execute dengine_draw_sequence_start,draw and end
 * \param primitive Primitive to draw
 * \param shader Shader to use
 */
void dengine_draw_primitive(const Primitive* primitive, const Shader* shader);

/*!
 * \brief Start sequence
 * \param primitive
 * \param shader
 */
void dengine_draw_sequence_start(const Primitive* primitive, const Shader* shader);

/*!
 * \brief Draw already started sequence
 */
void dengine_draw_sequence_draw();

/*!
 * \brief End started sequence
 */
void dengine_draw_sequence_end();

#ifdef __cplusplus
}
#endif
#endif // DRAW_H
