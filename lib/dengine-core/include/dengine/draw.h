#ifndef DRAW_H
#define DRAW_H

#include "primitive.h" //draw primitive
#include "shader.h" //use_program
#include <stdint.h> //uint32_t

#ifdef __cplusplus
extern "C" {
#endif

void dengine_draw_primitive(Primitive* primitive, Shader* shader);

#ifdef __cplusplus
}
#endif
#endif // DRAW_H
