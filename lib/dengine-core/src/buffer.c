#include "buffer.h"


#include "loadgl.h" //glGen,Bind,Data
void dengine_buffer_gen(size_t count, Buffer* buffers)
{
    for(size_t i = 0; i < count; i++)
        glGenBuffers(1, &buffers[i].buffer_id); DENGINE_CHECKGL
}

void dengine_buffer_bind(uint32_t target, Buffer* buffer)
{
    if(buffer)
        glBindBuffer(target, buffer->buffer_id);
    else
        glBindBuffer(target, 0);

    DENGINE_CHECKGL
}

void dengine_buffer_data(uint32_t target, Buffer* buffer)
{
    glBufferData(target, buffer->size, buffer->data, buffer->usage); DENGINE_CHECKGL
}
