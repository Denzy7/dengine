#include "dengine/vertex.h"

#include "dengine/loadgl.h"

void dengine_vertex_attribute_setup(VertexAttribute* attribute)
{
    glVertexAttribPointer(attribute->index,
                           attribute->size,
                           attribute->type,
                           attribute->normalize,
                           attribute->stride,
                           attribute->ptr);
    DENGINE_CHECKGL;
}

void dengine_vertex_attribute_enable(VertexAttribute* attribute)
{
    glEnableVertexAttribArray(attribute->index); DENGINE_CHECKGL;
}

void dengine_vertex_attribute_indexfromshader(Shader* shader, VertexAttribute* attribute, const char* attribute_name)
{
    int location = glGetAttribLocation(shader->program_id, attribute_name); DENGINE_CHECKGL;
    attribute->index = location;
}
