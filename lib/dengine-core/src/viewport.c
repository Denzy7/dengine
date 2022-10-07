#include "dengine/viewport.h"
#include "dengine/loadgl.h" //glGet, glViewport

#include "dengine-utils/debug.h"
void dengine_viewport_get(int* x, int* y, int* width, int* height)
{
    DENGINE_DEBUG_ENTER;

    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    DENGINE_CHECKGL;

    if(x)
        *x = viewport[0];
    if(y)
        *y = viewport[1];
    if(width)
        *width = viewport[2];
    if(height)
        *height = viewport[3];

}

void dengine_viewport_set(int x, int y, int width, int height)
{
    DENGINE_DEBUG_ENTER;

    glViewport(x, y, width, height);
    DENGINE_CHECKGL;
}
