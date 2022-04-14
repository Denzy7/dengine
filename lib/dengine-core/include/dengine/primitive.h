/*!
 * \file primitive,h
 * Execute Primitive operations
 */

#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include "dengine/buffer.h"
#include "dengine/vertex.h"
#include "dengine/vao.h"

#include <stdint.h> //uint32

/*! \struct Primitive
 *
 * A primitive can be seen as a definition of how an object is drawn.
 * You can think of it as a 3D mesh with a collection of 3D vertices.
 *
 * However there is more to it such as what its texture coordinates(aTexCoord),
 * normals (aNormal)
 *
 * Don't confuse this with GL primitives
 */
typedef struct
{
    VAO vao; /*!< Vertex array object for Primitive */

    Buffer array; /*!< Array buffer of Primitive */
    Buffer index; /*!< Main index buffer for how array buffer is drawn */
    uint16_t index_count; /*!< Number of indices in the main index buffer */
    void* offset; /*!< Offset at which GL will start drawing from in the index buffer. Like sizeof(type) * offset. Default is NULL */

    VertexAttribute aPos; /*!< VertexAtrribute for vertex position */
    VertexAttribute aTexCoord; /*!< VertexAtrribute for texture coordinate */
    VertexAttribute aNormal; /*!< VertexAtrribute for vertex normal */
    VertexAttribute aTangent; /*!< VertexAtrribute for tangent to normal */
    VertexAttribute aBiTangent; /*!< VertexAtrribute for cross of tangent and normal*/

    uint32_t draw_mode; /*!< How primitive is drawn with GL primitives. GL_TRIANGLES, GL_LINES,... */
    uint32_t draw_type; /*!< The type in the index buffer */
} Primitive;

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Setup a primitive with the help of a shader
 *
 * This sets up a vao, array buffer, index buffer and enables vertex attributes if they
 * are found in the shader.
 *
 * \param primitive Primitive to use
 * \param shader Shader to use
 */
void dengine_primitive_setup(Primitive* primitive, Shader* shader);

/*!
 * \brief Release resources being used by a set up primitive
 * \param primitive
 */
void dengine_primitive_destroy(Primitive* primitive);

/*!
 * \brief Generate an NDC fullscreen quad
 * \param primitive Primitive to use
 * \param shader Shader to use
 */
void dengine_primitive_gen_quad(Primitive* primitive, Shader* shader);

/*!
 * \brief Generate a 3D plane
 * \param primitive Primitive to use
 * \param shader Shader to use
 */
void dengine_primitive_gen_plane(Primitive* primitive, Shader* shader);

/*!
 * \brief Generate a 3D cube
 * \param primitive Primitive to use
 * \param shader Shader to use
 */
void dengine_primitive_gen_cube(Primitive* primitive, Shader* shader);

/*!
 * \brief Generates a plane then cuts it into \slice slices
 * \param slice Number of slices
 * \param primitive Primitive to use
 * \param shader Shader to use
 */
void dengine_primitive_gen_grid(uint16_t slice, Primitive* primitive, Shader* shader);

/*!
 * \brief Generate a 3D axis pointing at positive X, Y and Z
 *
 * By befault you get 6 index counts which draw an axis with same color.
 * To vary the color, change the index count to 2 then use a for loop to add the offset
 * +2 then set the uniform color as desired
 *
 * float color[3];
 * for (int i = 0; i < 3; i++) {
 * color[0] = i == 0 ? 1.0f : 0.0f, color[1] = i == 1 ? 1.0f : 0.0f, color[2] = i == 2 ? 1.0f : 0.0f;
 * axis->offset = (void*)(i*2*sizeof (uint16_t));
 * dengine_shader_set_vec3(shader, "color", color);
 * }
 *
 * \param primitive Primitive to use
 * \param shader Shader to use
 */
void dengine_primitive_gen_axis(Primitive* primitive, Shader* shader);

#ifdef __cplusplus
}
#endif

#endif // PRIMITIVE_H
