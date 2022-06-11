#include "dengine/draw.h"

#include "dengine/loadgl.h"
#include "dengine/entrygl.h"
#include "dengine/buffer.h" //bind

void dengine_draw_primitive(const Primitive* primitive, const Shader* shader)
{
    Buffer entry_vbo, entry_ibo;
    VAO entry_vao;
    Shader entry_shader;
    int prof;
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &prof);

    if(prof == GL_CONTEXT_CORE_PROFILE_BIT)
    {
        dengine_entrygl_vao(&entry_vao );
        dengine_vao_bind(&primitive->vao);
    }else
    {
        // get entry stuff
        dengine_entrygl_buffer(GL_ARRAY_BUFFER, &entry_vbo);
        dengine_entrygl_buffer(GL_ELEMENT_ARRAY_BUFFER, &entry_ibo );

        dengine_buffer_bind(GL_ARRAY_BUFFER, &primitive->array);
        dengine_buffer_bind(GL_ELEMENT_ARRAY_BUFFER, &primitive->index);

        //compat needs attribs enabled before draw
        dengine_primitive_attributes_enable(primitive, shader);
    }

    dengine_entrygl_shader(&entry_shader );
    dengine_shader_use(shader);

    //Don't draw if we have an ERROR. Prevent's infinite spawn of dialogs
    glDrawElements(primitive->draw_mode, primitive->index_count, primitive->draw_type, primitive->offset);
    DENGINE_CHECKGL;

    dengine_shader_use(&entry_shader);

    if(prof == GL_CONTEXT_CORE_PROFILE_BIT)
    {
        dengine_vao_bind(&entry_vao);
    }else
    {
        dengine_buffer_bind(GL_ARRAY_BUFFER, &entry_vbo);
        dengine_buffer_bind(GL_ELEMENT_ARRAY_BUFFER, &entry_ibo);
    }
}
