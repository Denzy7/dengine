#include "draw.h"

#include "loadgl.h"

#include "buffer.h" //bind
void dengine_draw_primitive(Primitive* primitive, Shader* shader)
{
    dengine_buffer_bind(GL_ARRAY_BUFFER, &primitive->array);
    dengine_buffer_bind(GL_ELEMENT_ARRAY_BUFFER, &primitive->index);

    dengine_shader_use(shader);

    glDrawElements(primitive->draw_mode, primitive->index_count, primitive->draw_type, 0);
    //DRAW ERROR WILL CAUSE INFINITE SPAWN OF DIALOGS!
    DENGINE_CHECKGL

    dengine_buffer_bind(GL_ARRAY_BUFFER, NULL);
    dengine_buffer_bind(GL_ELEMENT_ARRAY_BUFFER, NULL);

    dengine_shader_use(NULL);
}
