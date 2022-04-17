#include "dengine/entrygl.h"
#include "dengine/loadgl.h"
#include "dengine-utils/logging.h"

void dengine_entrygl_texture(uint32_t target, Texture* texture)
{
    if(target == GL_TEXTURE_2D)
        glGetIntegerv(GL_TEXTURE_BINDING_2D, (int*) &texture->texture_id);
    else if(target == GL_TEXTURE_CUBE_MAP)
        glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, (int*) &texture->texture_id);
    else
        dengineutils_logging_log("ERROR::ENTRY_GL : invalid texture target");

    DENGINE_CHECKGL;
}

void dengine_entrygl_texture_active(int* texture_active)
{
    glGetIntegerv(GL_ACTIVE_TEXTURE, texture_active);

    DENGINE_CHECKGL;
}

void dengine_entrygl_framebuffer(uint32_t target, Framebuffer* framebuffer)
{
    if(target == GL_FRAMEBUFFER)
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, (int*) &framebuffer->framebuffer_id);
    else if(target == GL_DRAW_FRAMEBUFFER)
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, (int*) &framebuffer->framebuffer_id);
    else if(target == GL_READ_FRAMEBUFFER)
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, (int*) &framebuffer->framebuffer_id);
    else
        dengineutils_logging_log("ERROR::ENTRY_GL : invalid framebuffer target");

    DENGINE_CHECKGL;
}

void dengine_entrygl_buffer(uint32_t target, Buffer* buffer)
{
    if(target == GL_ARRAY_BUFFER)
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, (int*) &buffer->buffer_id);
    else if(target == GL_ELEMENT_ARRAY_BUFFER)
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, (int*) &buffer->buffer_id);
    else if(target == GL_UNIFORM_BUFFER)
        glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, (int*) &buffer->buffer_id);
    else
        dengineutils_logging_log("ERROR::ENTRY_GL : invalid buffer target");

    DENGINE_CHECKGL;
}

void dengine_entrygl_vao(VAO* vao)
{
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, (int*) &vao->vao);

    DENGINE_CHECKGL;
}
