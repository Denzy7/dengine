#ifndef DENGINESCENE_SCRIPTINGSYNC_PYTHON_ENTITY_H
#define DENGINESCENE_SCRIPTINGSYNC_PYTHON_ENTITY_H

#include "dengine-scene/ecs.h"
#include "dengine-scene/scriptingsync/scriptingsync.h"
#include "dengine-script/python/modules/scenemodule.h"
#include "dengine-script/script.h"

#ifdef __cplusplus
extern "C" {
#endif

/* push data from Entity* into EntityObject. scenemodule can create EntityObject for you, free of charge*/
void denginescene_scriptingsync_python_push(ScriptingSyncFlags flags, const Entity* entity, EntityObject* pyobject);

/* pull data from EntityObject into Entity* */
void denginescene_scriptingsync_python_pull(ScriptingSyncFlags flags, Entity* entity, const EntityObject* pyobject);

#ifdef __cplusplus
}
#endif
#endif
