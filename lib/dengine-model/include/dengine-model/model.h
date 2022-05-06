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

Primitive* denginemodel_load_file(DengineModelFormat format, const char* file, size_t* meshes , Shader* shader);

Primitive* denginemodel_load_mem(DengineModelFormat format, const void* mem, const size_t sz, size_t* meshes, Shader* shader);

#ifdef __cplusplus
}
#endif
#endif
