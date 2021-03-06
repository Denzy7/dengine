/*! \file vao.h
 *  Execute vertex array object (VAO) operations
 */

#ifndef VAO_H
#define VAO_H

#include <stdint.h> //uint32
#include <stddef.h> //size_t

/*! \struct VAO defines how data is dispatched to GL
 *
 *  VAO's are needed for core GL (3.2+). But older GL can do without
 */
typedef struct VAO
{
    uint32_t vao;
}VAO;

#ifdef __cplusplus
extern "C" {
#endif


/*!
 * \brief Generate a VAO (only for GL or ES 3.2)
 * \param count Number of VAO's to generate
 * \param vaos Pointer to array of VAO's
 */
void dengine_vao_gen(size_t count, VAO* vaos);

/*!
 * \brief Release a VAO (only for GL or ES 3.2)
 * \param count Number of VAOs
 * \param vaos VAOs
 */
void dengine_vao_destroy(size_t count, VAO* vaos);

/*!
 * \brief Bind a VAO (only for GL or ES 3.2)
 * \param vao VAO to bind
 */
void dengine_vao_bind(const VAO* vao);

#ifdef __cplusplus
}
#endif

#endif // VAO_H
