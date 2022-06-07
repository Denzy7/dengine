#include "dengine/vao.h"

#include "dengine/loadgl.h"

void dengine_vao_gen(size_t count, VAO* vaos)
{
    for(size_t i = 0; i < count; i++)
    {
        if(glad_glGenVertexArrays)
            glGenVertexArrays(1, &vaos[i].vao);
        DENGINE_CHECKGL;
    }
}

void dengine_vao_bind(const VAO* vao)
{

    uint32_t _vao = 0;
    if(vao)
        _vao = vao->vao;

    if(glad_glBindVertexArray)
        glBindVertexArray(_vao);

    DENGINE_CHECKGL;

}

void dengine_vao_destroy(size_t count, VAO* vaos)
{
    for(size_t i = 0; i < count; i++)
    {
        if(glad_glDeleteVertexArrays)
            glDeleteVertexArrays(1, &vaos[i].vao);

        DENGINE_CHECKGL;
    }
}
