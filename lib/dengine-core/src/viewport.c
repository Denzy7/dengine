#include "dengine/viewport.h"
#include "dengine/loadgl.h" //glGet, glViewport

void dengine_viewport_get(int* x, int* y, int* width, int* height)
{
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

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
    glViewport(x, y, width, height);
}
