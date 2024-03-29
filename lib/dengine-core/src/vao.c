#include "dengine/vao.h"

#include "dengine/loadgl.h"

#include "dengine-utils/logging.h"
#include "dengine-utils/debug.h"
void dengine_vao_gen(size_t count, VAO* vaos)
{
    DENGINE_DEBUG_ENTER;

    for(size_t i = 0; i < count; i++)
    {
        if(glad_glGenVertexArrays)
            glGenVertexArrays(1, &vaos[i].vao);
        else
            dengineutils_logging_log("WARNING::glad_glGenVertexArrays not found!");
        DENGINE_CHECKGL;
    }
}

void dengine_vao_bind(const VAO* vao)
{
    DENGINE_DEBUG_ENTER;

    uint32_t _vao = 0;
    if(vao)
        _vao = vao->vao;

    if(glad_glBindVertexArray)
        glBindVertexArray(_vao);
    else
        dengineutils_logging_log("WARNING::glad_glGenVertexArrays not found!");

    DENGINE_CHECKGL;

}

void dengine_vao_destroy(size_t count, VAO* vaos)
{
    DENGINE_DEBUG_ENTER;

    for(size_t i = 0; i < count; i++)
    {
        if(glad_glDeleteVertexArrays)
            glDeleteVertexArrays(1, &vaos[i].vao);
        else
            dengineutils_logging_log("WARNING::glad_glGenVertexArrays not found!");

        DENGINE_CHECKGL;
    }
}
