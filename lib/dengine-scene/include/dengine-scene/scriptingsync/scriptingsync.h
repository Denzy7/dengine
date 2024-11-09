#ifndef DENGINESCENE_SCRIPTINGSYNC_ENTITY_H
#define DENGINESCENE_SCRIPTINGSYNC_ENTITY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "dengine-script/script.h"
typedef enum
{
    DENGINESCENE_SCRIPTSYNC_TRANSFORM = 1,
    DENGINESCENE_SCRIPTSYNC_CAMERA = 1 << 1,
    DENGINESCENE_SCRIPTSYNC_MESH = 1 << 2,
    DENGINESCENE_SCRIPTSYNC_LIGHT = 1 << 3,

    DENGINESCENE_SCRIPTSYNC_ALL = UINTMAX_MAX
}ScriptingSyncFlags;

#ifdef __cplusplus
}
#endif

#endif


