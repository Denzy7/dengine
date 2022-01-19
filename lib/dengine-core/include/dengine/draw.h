#ifndef DRAW_H
#define DRAW_H

#include "primitive.h" //draw primitive
#include "shader.h" //use_program
#include <stdint.h> //uint32_t

void dengine_draw_primitive(Primitive* primitive, Shader* shader);

#endif // DRAW_H
