#ifndef VIEWPORT_H
#define VIEWPORT_H

#ifdef __cplusplus
extern "C" {
#endif

void dengine_viewport_get(int* x, int* y, int* width, int* height);

void dengine_viewport_set(int x, int y, int width, int height);

#ifdef __cplusplus
}
#endif

#endif // VIEWPORT_H
