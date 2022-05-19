/*! \file buffer.h
 *  Execute buffer operations
 */

#ifndef BUFFER_H
#define BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h> //uint32
#include <stddef.h> //size_t

/*! \struct Buffer
 *  Buffer struct with data to pass to OpenGL
 */
typedef struct{
    uint32_t buffer_id; /*!< Buffer ID. Don't change this */
    void* data; /*!< Buffer data pointer */
    size_t size; /*!< Buffer size of pointer */
    uint32_t usage; /*!< Usage. GL_STATIC_DRAW OR GL_DYNAMIC_DRAW */
} Buffer;

/*!
 * \brief Generate some buffers
 * \param count The number of buffers in buffer pointer
 * \param buffers Pointer allocated with Buffer struct
 */
void dengine_buffer_gen(const size_t count, Buffer* buffers);

/*!
 * \brief Bind a buffer
 * \param target Can be of GL_UNIFORM, GL_ARRAY, GL_INDEX, ....
 * \param buffer Buffer to bind. NULL unbinds the currently bound buffer in target
 */
void dengine_buffer_bind(const uint32_t target, const Buffer* buffer);

/*!
 * \brief Update data in buffer to OpenGL
 * \param target Can be of GL_UNIFORM, GL_ARRAY, GL_INDEX, ....
 * \param buffer Buffer to update
 */
void dengine_buffer_data(const uint32_t target, const Buffer* buffer);

/*!
 * \brief Release resources used by a buffer
 * \param count Number of buffers
 * \param buffers Pointer to the buffers
 */
void dengine_buffer_destroy(const size_t count, Buffer* buffers);

#ifdef __cplusplus
}
#endif

#endif // BUFFER_H
