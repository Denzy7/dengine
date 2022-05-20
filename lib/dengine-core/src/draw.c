#include "dengine/draw.h"

#include "dengine/loadgl.h"
#include "dengine/entrygl.h"
#include "dengine/buffer.h" //bind

void dengine_draw_primitive(const Primitive* primitive, const Shader* shader)
{
    Buffer entry_vbo, entry_ibo;
    VAO entry_vao;
    Shader entry_shader;

    // get entry stuff
    dengine_entrygl_buffer(GL_ARRAY_BUFFER, &entry_vbo);
    dengine_entrygl_buffer(GL_ELEMENT_ARRAY_BUFFER, &entry_ibo );
    dengine_entrygl_vao(&entry_vao );
    dengine_entrygl_shader(&entry_shader );

    //Don't draw if we have an ERROR. Prevent's infinite spawn of dialogs
    dengine_vao_bind(&primitive->vao);

    dengine_buffer_bind(GL_ARRAY_BUFFER, &primitive->array);
    dengine_buffer_bind(GL_ELEMENT_ARRAY_BUFFER, &primitive->index);

    dengine_shader_use(shader);

    glDrawElements(primitive->draw_mode, primitive->index_count, primitive->draw_type, primitive->offset);
    DENGINE_CHECKGL;

    dengine_buffer_bind(GL_ARRAY_BUFFER, &entry_vbo);
    dengine_buffer_bind(GL_ELEMENT_ARRAY_BUFFER, &entry_ibo);

    dengine_shader_use(&entry_shader);

    dengine_vao_bind(&entry_vao);
}
