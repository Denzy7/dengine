#include "dengine/draw.h"

#include "dengine/loadgl.h"
#include "dengine/entrygl.h"
#include "dengine/buffer.h" //bind

#include "dengine-utils/debug.h"
#include "dengine/primitive.h"

Buffer entry_vbo_seq, entry_ibo_seq;
VAO entry_vao_seq;
Shader entry_shader_seq;
const Primitive* current_primitive;

void dengine_draw_sequence_start(const Primitive* primitive, const Shader* shader)
{
    DENGINE_DEBUG_ENTER;

    if(GLAD_GL_VERSION_3_2 || GLAD_GL_ES_VERSION_3_2)
    {
        if(dengine_entrygl_get_enabled())
            dengine_entrygl_vao(&entry_vao_seq );
        dengine_vao_bind(&primitive->vao);
    }else
    {
        // get entry stuff
        if(dengine_entrygl_get_enabled()){
            dengine_entrygl_buffer(GL_ARRAY_BUFFER, &entry_vbo_seq);
            dengine_entrygl_buffer(GL_ELEMENT_ARRAY_BUFFER, &entry_ibo_seq );
        }

        dengine_buffer_bind(GL_ARRAY_BUFFER, &primitive->array);
        dengine_buffer_bind(GL_ELEMENT_ARRAY_BUFFER, &primitive->index);

        //compat needs attribs enabled before draw
        dengine_primitive_attributes_enable(primitive, shader);
    }
    if(dengine_entrygl_get_enabled())
        dengine_entrygl_shader(&entry_shader_seq );

    current_primitive = primitive;
    dengine_shader_use(shader);
}

void dengine_draw_sequence_draw()
{
    //Don't draw if we have an ERROR. Prevent's infinite thrashing of stdout
    glDrawElements(current_primitive->draw_mode, current_primitive->index_count, current_primitive->draw_type, current_primitive->offset);
    DENGINE_CHECKGL;
}

void dengine_draw_sequence_end()
{
    if(dengine_entrygl_get_enabled())
    {
        dengine_shader_use(&entry_shader_seq);

        if(GLAD_GL_VERSION_3_2 || GLAD_GL_ES_VERSION_3_2)
        {
            dengine_vao_bind(&entry_vao_seq);
        }else
        {
            dengine_buffer_bind(GL_ARRAY_BUFFER, &entry_vbo_seq);
            dengine_buffer_bind(GL_ELEMENT_ARRAY_BUFFER, &entry_ibo_seq);
        }
    }
}

void dengine_draw_primitive(const Primitive* primitive, const Shader* shader)
{
    dengine_draw_sequence_start(primitive, shader);
    dengine_draw_sequence_draw();
    dengine_draw_sequence_end();
}
