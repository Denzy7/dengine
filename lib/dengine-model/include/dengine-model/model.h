#include "dengine/primitive.h"

#include <stdint.h> //uint16,32

typedef enum DengineModelFormat
{
     DENGINE_MODEL_FORMAT_OBJ
}DengineModelFormat;

int denginemodel_load_file(DengineModelFormat format, const char* file, Primitive* primitive, Shader* shader);
