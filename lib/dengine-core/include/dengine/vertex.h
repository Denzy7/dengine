/*! \file vertex.h
 *  Execute vertex commands
 */
#ifndef VERTEX_H
#define VERTEX_H

#include <stddef.h> //size_t
#include <stdint.h> //uint32

#include "shader.h" //Shader

/*!
 * \struct VertexAttribute
 * Define a vertex attribute
 *
 * A VertexAttribute tells GL how an array buffer (in most cases) is structured.
 * Attributes can be data passed to GL shader like positions, normal, texture coordinates etc.
 *
 * Attributes are typically passed when packed to avoid repetition such as having
 * <posx_1>,<posy_1>,<posz_1>,<texcoordx_1>,<texcoordy_1>,..., --> collection_1
 * ...
 * <posx_n>,<posy_n>,<posz_n>,<texcoordx_n>,<texcoordy_n>,..., --> collection_n
 *
 * A collection is created when an entire row of packed attributes form a vertex definition
 */
typedef struct VertexAttribute
{
    uint32_t index; /*!< Index of attrib. Automatically got from shader */
    int size; /*!< Size/components in attribute */
    uint32_t type; /*!< Type of attribute. Typically FLOAT */
    uint32_t normalize; /*!< Normalize? Typically false */
    size_t stride; /*!< How long is the entire collection of one attribute in the array buffer */
    void* ptr; /*!< Offset of the specific attribute we want in that collection */
}VertexAttribute;

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Define the vertex attribute
 * \param attribute VertexAttribute to define
 */
void dengine_vertex_attribute_setup(VertexAttribute* attribute);

/*!
 * \brief Enable a vertex attribute
 * \param attribute VertexAttribute to enable
 */
void dengine_vertex_attribute_enable(VertexAttribute* attribute);

/*!
 * \brief Get the index of an attribute from a shader
 * \param shader Shader to use
 * \param attribute Attribute to search
 * \param attribute_name Attribute name in shader
 */
void dengine_vertex_attribute_indexfromshader(Shader* shader, VertexAttribute* attribute, const char* attribute_name);

#ifdef __cplusplus
}
#endif

#endif // VERTEXARRAY_H
