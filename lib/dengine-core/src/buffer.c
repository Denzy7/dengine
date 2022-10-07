#include "dengine/buffer.h"
#include "dengine-utils/debug.h"

#include "dengine/loadgl.h" //glGen,Bind,Data
void dengine_buffer_gen(const size_t count, Buffer* buffers)
{
    DENGINE_DEBUG_ENTER;

    for(size_t i = 0; i < count; i++)
    {glGenBuffers(1, &buffers[i].buffer_id); DENGINE_CHECKGL;}
}

void dengine_buffer_bind(const uint32_t target, const Buffer* buffer)
{
    DENGINE_DEBUG_ENTER;

    if(buffer)
        glBindBuffer(target, buffer->buffer_id);
    else
        glBindBuffer(target, 0);

    DENGINE_CHECKGL;
}

void dengine_buffer_data(const uint32_t target, const Buffer* buffer)
{
    DENGINE_DEBUG_ENTER;

    glBufferData(target, buffer->size, buffer->data, buffer->usage); DENGINE_CHECKGL;
}

void dengine_buffer_destroy(const size_t count, Buffer* buffers)
{
    DENGINE_DEBUG_ENTER;

    for(size_t i = 0; i < count; i++)
    {glDeleteBuffers(1, &buffers[i].buffer_id); DENGINE_CHECKGL;}
}
