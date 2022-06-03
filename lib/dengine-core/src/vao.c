#include "dengine/vao.h"

#include <string.h> //memset

#include "dengine/loadgl.h"
void dengine_vao_gen(size_t count, VAO* vaos)
{
    for(size_t i = 0; i < count; i++)
    {
        //Check for possible PFNs
        if(glad_glGenVertexArrays)
            glGenVertexArrays(1, &vaos[i].vao);
        else if(glad_glGenVertexArraysOES)
            glGenVertexArraysOES(1, &vaos[i].vao);
        else if(glad_glGenVertexArraysAPPLE)
            glGenVertexArraysAPPLE(1, &vaos[i].vao);

        DENGINE_CHECKGL;
    }
}

void dengine_vao_bind(const VAO* vao)
{

    uint32_t _vao = 0;
    if(vao)
        _vao = vao->vao;

    //Check for possible PFNs
    if(glad_glBindVertexArray)
        glBindVertexArray(_vao);
    else if(glad_glBindVertexArrayOES)
        glBindVertexArrayOES(_vao);
    else if(glad_glBindVertexArrayAPPLE)
        glBindVertexArrayAPPLE(_vao);

    DENGINE_CHECKGL;

}

void dengine_vao_destroy(size_t count, VAO* vaos)
{
    for(size_t i = 0; i < count; i++)
    {
        //Check for possible PFNs
        if(glad_glDeleteVertexArrays)
            glDeleteVertexArrays(1, &vaos[i].vao);
        else if(glad_glDeleteVertexArrays)
            glDeleteVertexArraysOES(1, &vaos[i].vao);
        else if(glad_glDeleteVertexArraysAPPLE)
            glDeleteVertexArraysAPPLE(1, &vaos[i].vao);

        DENGINE_CHECKGL;
    }
}
