#ifndef DGL_H
#define DGL_H

#include <stdint.h>

typedef uint32_t GLenum;
typedef uint8_t GLboolean;
typedef uint32_t GLbitfield;
typedef void GLvoid;
typedef int8_t GLbyte;
typedef uint8_t GLubyte;
typedef int8_t GLshort;
typedef uint8_t GLushort;
typedef int32_t GLint;
typedef uint32_t GLuint;
typedef int32_t GLsizei;
typedef float GLfloat;
typedef double GLdouble;
typedef char GLchar;
typedef ssize_t GLsizeiptr;

typedef const GLubyte* (*dgl_GetString)(GLenum name);
static dgl_GetString glGetString;
typedef GLenum (*dgl_GetError)();
static dgl_GetError glGetError;
typedef void (*dgl_GetIntegerv)(GLenum pname, GLint* data);
static dgl_GetIntegerv glGetIntegerv;
typedef void (*dgl_GetFloatv)(GLenum pname, GLfloat* data);
static dgl_GetFloatv glGetFloatv;

typedef void (*dgl_Clear)(GLbitfield mask);
static dgl_Clear glClear;
typedef void (*dgl_ClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
static dgl_ClearColor glClearColor;

typedef void (*dgl_GenBuffers)(GLsizei n, GLuint* buffers);
static dgl_GenBuffers glGenBuffers;
static dgl_GenBuffers glDeleteBuffers;
typedef void(*dgl_BufferData)(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
static dgl_BufferData glBufferData;
typedef void(*dgl_BindBuffer)(GLenum target, GLuint buffer);
static dgl_BindBuffer glBindBuffer;

typedef void (*dgl_DrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices);
static dgl_DrawElements glDrawElements;

typedef void (*dgl_DrawArrays)(GLenum mode, GLint first, GLsizei count);
static dgl_DrawArrays glDrawArrays;

/*
GL_COLOR_BUFFER_BIT
GL_DEPTH_BUFFER_BIT

GL_VIEWPORT

GL_CLAMP_TO_EDGE

GL_TEXTURE_2D
GL_TEXTURE_BINDING_2D

GL_TEXTURE_CUBE_MAP_POSITIVE_X
GL_TEXTURE_CUBE_MAP
GL_TEXTURE_BINDING_CUBE_MAP

GL_READ_FRAMEBUFFER
GL_READ_FRAMEBUFFER_BINDING
GL_FRAMEBUFFER
GL_FRAMEBUFFER_BINDING

GL_LINEAR
GL_NEAREST

GL_RGBA
GL_RGB
GL_DEPTH_COMPONENT

GL_UNSIGNED_BYTE
GL_UNSIGNED_SHORT

GL_ARRAY_BUFFER
GL_ARRAY_BUFFER_BINDING
GL_ELEMENT_ARRAY_BUFFER
GL_ELEMENT_ARRAY_BUFFER_BINDING
GL_UNIFORM_BUFFER
GL_UNIFORM_BUFFER_BINDING

GL_VERTEX_ARRAY_BINDING
GL_CURRENT_PROGRAM

GL_ACTIVE_TEXTURE
*/

/*
glReadPixels
glFinish
glFramebufferRenderbuffer
glBindFramebuffer
glFramebufferTexture
glFramebufferTexture2D
*/

int dgl_load();

#endif // DGL_H
