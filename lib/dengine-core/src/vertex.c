#include "dengine/vertex.h"

#include "dengine/loadgl.h"
#include "dengine-utils/debug.h"
void dengine_vertex_attribute_setup(const VertexAttribute* attribute)
{
    DENGINE_DEBUG_ENTER;

    glVertexAttribPointer(attribute->index,
                           attribute->size,
                           attribute->type,
                           attribute->normalize,
                           attribute->stride,
                           attribute->ptr);
    DENGINE_CHECKGL;
}

void dengine_vertex_attribute_enable(const VertexAttribute* attribute)
{
    DENGINE_DEBUG_ENTER;

    glEnableVertexAttribArray(attribute->index); DENGINE_CHECKGL;
}

void dengine_vertex_attribute_indexfromshader(const Shader* shader, VertexAttribute* attribute, const char* attribute_name)
{
    DENGINE_DEBUG_ENTER;

    int location = glGetAttribLocation(shader->program_id, attribute_name); DENGINE_CHECKGL;
    attribute->index = location;
}
