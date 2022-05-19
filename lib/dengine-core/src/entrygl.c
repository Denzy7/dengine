#include "dengine/entrygl.h"
#include "dengine/loadgl.h"
#include "dengine-utils/logging.h"

Texture entrygl_texture;
Framebuffer entrygl_framebuffer;
Buffer entrygl_buffer;
VAO entrygl_vao;
Shader entrygl_shader;
int entrygl_texture_active;

const Texture* dengine_entrygl_texture(uint32_t target)
{
    if(target == GL_TEXTURE_2D)
        glGetIntegerv(GL_TEXTURE_BINDING_2D, (int*) &entrygl_texture.texture_id);
    else if(target == GL_TEXTURE_CUBE_MAP)
        glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, (int*) &entrygl_texture.texture_id);
    else
        dengineutils_logging_log("ERROR::ENTRY_GL : invalid texture target");

    DENGINE_CHECKGL;

    return &entrygl_texture;
}

const int dengine_entrygl_texture_active()
{
    glGetIntegerv(GL_ACTIVE_TEXTURE, &entrygl_texture_active);

    DENGINE_CHECKGL;
    return entrygl_texture_active;
}

const Framebuffer* dengine_entrygl_framebuffer(uint32_t target)
{
    if(target == GL_FRAMEBUFFER)
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, (int*) &entrygl_framebuffer.framebuffer_id);
    else if(target == GL_DRAW_FRAMEBUFFER)
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, (int*) &entrygl_framebuffer.framebuffer_id);
    else if(target == GL_READ_FRAMEBUFFER)
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, (int*) &entrygl_framebuffer.framebuffer_id);
    else
        dengineutils_logging_log("ERROR::ENTRY_GL : invalid framebuffer target");

    DENGINE_CHECKGL;

    return &entrygl_framebuffer;
}

const Buffer* dengine_entrygl_buffer(uint32_t target)
{
    if(target == GL_ARRAY_BUFFER)
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, (int*) &entrygl_buffer.buffer_id);
    else if(target == GL_ELEMENT_ARRAY_BUFFER)
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, (int*) &entrygl_buffer.buffer_id);
    else if(target == GL_UNIFORM_BUFFER)
        glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, (int*) &entrygl_buffer.buffer_id);
    else
        dengineutils_logging_log("ERROR::ENTRY_GL : invalid buffer target");

    DENGINE_CHECKGL;

    return &entrygl_buffer;
}

const VAO* dengine_entrygl_vao()
{
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, (int*) &entrygl_vao.vao);
    DENGINE_CHECKGL;
    return &entrygl_vao;
}

const Shader* dengine_entrygl_shader()
{
    glGetIntegerv(GL_CURRENT_PROGRAM, (int*) &entrygl_shader.program_id);

    DENGINE_CHECKGL;
    return &entrygl_shader;
}
