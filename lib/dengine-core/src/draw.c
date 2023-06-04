#include "dengine/draw.h"

#include "dengine/loadgl.h"
#include "dengine/entrygl.h"
#include "dengine/buffer.h" //bind

#include "dengine-utils/debug.h"

void dengine_draw_primitive(const Primitive* primitive, const Shader* shader)
{
    DENGINE_DEBUG_ENTER;

    Buffer entry_vbo, entry_ibo;
    VAO entry_vao;
    Shader entry_shader;

    if(GLAD_GL_VERSION_3_2 || GLAD_GL_ES_VERSION_3_2)
    {
        if(dengine_entrygl_get_enabled())
            dengine_entrygl_vao(&entry_vao );
        dengine_vao_bind(&primitive->vao);
    }else
    {
        // get entry stuff
        if(dengine_entrygl_get_enabled()){
            dengine_entrygl_buffer(GL_ARRAY_BUFFER, &entry_vbo);
            dengine_entrygl_buffer(GL_ELEMENT_ARRAY_BUFFER, &entry_ibo );
        }

        dengine_buffer_bind(GL_ARRAY_BUFFER, &primitive->array);
        dengine_buffer_bind(GL_ELEMENT_ARRAY_BUFFER, &primitive->index);

        //compat needs attribs enabled before draw
        dengine_primitive_attributes_enable(primitive, shader);
    }
    if(dengine_entrygl_get_enabled())
        dengine_entrygl_shader(&entry_shader );

    dengine_shader_use(shader);

    //Don't draw if we have an ERROR. Prevent's infinite thrashing of stdout
    glDrawElements(primitive->draw_mode, primitive->index_count, primitive->draw_type, primitive->offset);
    DENGINE_CHECKGL;

    if(dengine_entrygl_get_enabled())
    {
        dengine_shader_use(&entry_shader);

        if(GLAD_GL_VERSION_3_2 || GLAD_GL_ES_VERSION_3_2)
        {
            dengine_vao_bind(&entry_vao);
        }else
        {
            dengine_buffer_bind(GL_ARRAY_BUFFER, &entry_vbo);
            dengine_buffer_bind(GL_ELEMENT_ARRAY_BUFFER, &entry_ibo);
        }
    }
}
