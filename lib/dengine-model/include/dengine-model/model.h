#ifndef MODEL_H
#define MODEL_H
#include "dengine/primitive.h"

#include <stdint.h> //uint16,32

typedef enum DengineModelFormat
{
     DENGINE_MODEL_FORMAT_OBJ
}DengineModelFormat;

#ifdef __cplusplus
extern "C" {
#endif

int denginemodel_load_file(DengineModelFormat format, const char* file, Primitive* primitive, Shader* shader);

#ifdef __cplusplus
}
#endif
#endif
