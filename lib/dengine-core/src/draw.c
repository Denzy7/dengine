#include "draw.h"

#include "loadgl.h"

#include "buffer.h" //bind

void dengine_draw_primitive(Primitive* primitive, Shader* shader)
{
    //Don't draw if we have an ERROR. Prevent's infinite spawn of dialogs
    if(!DENGINE_CHECKGL)
    {
        glBindVertexArray(primitive->vao);

        dengine_buffer_bind(GL_ARRAY_BUFFER, &primitive->array);
        dengine_buffer_bind(GL_ELEMENT_ARRAY_BUFFER, &primitive->index);

        dengine_shader_use(shader);

        glDrawElements(primitive->draw_mode, primitive->index_count, primitive->draw_type, 0);

        dengine_buffer_bind(GL_ARRAY_BUFFER, NULL);
        dengine_buffer_bind(GL_ELEMENT_ARRAY_BUFFER, NULL);

        dengine_shader_use(NULL);

        glBindVertexArray(0);
    }
}
