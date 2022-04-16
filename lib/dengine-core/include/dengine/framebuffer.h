/*! \file framebuffer.h
 *  Execute framebuffer operations
 */

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stddef.h> //size_t
#include <stdint.h> //uint32

#include "dengine/texture.h"
#include "dengine/renderbuffer.h"
//#include "dengine/loadgl.h"

/*! \enum FramebufferAttachmentType
 *  Helper for various framebuffer atttachments
 */
typedef enum
{
    DENGINE_FRAMEBUFFER_COLOR,// = GL_COLOR_ATTACHMENT0,
    DENGINE_FRAMEBUFFER_DEPTH,// = GL_DEPTH_ATTACHMENT,
    DENGINE_FRAMEBUFFER_STENCIL,// = GL_STENCIL_ATTACHMENT
}FramebufferAttachmentType;

/*! \struct Framebuffer
 *  struct describing a framebuffer
 */
typedef struct
{
    uint32_t framebuffer_id; /*!< Framebuffer id. DON'T change this */

    Texture color[8]; /*!< Attached color textures */
    uint32_t n_color; /*!< Number of attached color textures */

    Texture depth; /*!< Attached depth texture */
    Texture stencil; /*!< Attached stencil texture */
}Framebuffer;

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Generate some framebuffers
 * \param count Number of framebuffers
 * \param framebuffers Pointer with framebuffers
 */
void dengine_framebuffer_gen(size_t count, Framebuffer* framebuffers);

/*!
 * \brief Bind a framebuffer
 * \param target Can be GL_FRAMEBUFFER, GL_READBUFFER,...
 * \param framebuffer Framebuffer to bind
 */
void dengine_framebuffer_bind(uint32_t target, Framebuffer* framebuffer);

/*!
 * \brief Attach a texture to a framebuffer (GL/ES 3.2+)
 * \param attachment Which attachment point to use
 * \param texture Texture to attach
 * \param framebuffer Framebuffer to use
 *
 * \warning This call requires at least GL 3.2+ or will not succeed
 */
void dengine_framebuffer_attach(FramebufferAttachmentType attachment, Texture* texture, Framebuffer* framebuffer);

/*!
 * \brief Attach a 2D texture to a framebuffer
 * \param attachment Which attachment point to use
 * \param texture 2D texture to attach
 * \param framebuffer Framebuffer to use
 */
void dengine_framebuffer_attach2D(FramebufferAttachmentType attachment, Texture* texture, Framebuffer* framebuffer);

/*!
 * \brief Attach a Renderbuffer to a framebuffer
 * \param attachment attachment point to use
 * \param renderbuffer Renderbuffer to attach
 * \param framebuffer Framebuffer to use
 */
void dengine_framebuffer_attachRB(FramebufferAttachmentType attachment, Renderbuffer* renderbuffer, Framebuffer* framebuffer);

/*!
 * \brief Readback a framebuffer attachment to an allocated texture
 *
 * Note that this call will first bind the framebuffer, read from it then
 * bind the window framebuffer i.e. 0.
 *
 * Don't run this when you have a bound framebuffer as it will be unbound!
 *
 * This call is incredibly slow as it calls glFinish() to wait for readback to complete
 *
 * \param dest Texture to write to
 * \param framebuffer Framebuffer to read from
 */
void dengine_framebuffer_readback(Texture* dest, Framebuffer* framebuffer);

/*!
 * \brief Get the last glClearColor
 * \param r red
 * \param g green
 * \param b blue
 * \param a alpha
 */
void dengine_framebuffer_get_clearcolor(float* r, float* g, float* b, float* a);

#ifdef __cplusplus
}
#endif
#endif // FRAMEBUFFER_H
