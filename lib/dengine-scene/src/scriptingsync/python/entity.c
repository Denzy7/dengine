#include "dengine-scene/scriptingsync/python/entity.h"

/*TODO: figure out a way to efficiantly use flags ? */

void denginescene_scriptingsync_python_push(ScriptingSyncFlags flags, const Entity* entity, EntityObject* pyobject)
{
    for(size_t i = 0; i < 3; i++)
    {
        pyobject->transform->position->_xyzw[i] = entity->transform.position[i];
        pyobject->transform->rotation->_xyzw[i] = entity->transform.rotation[i];
        pyobject->transform->scale->_xyzw[i] = entity->transform.scale[i];
    }
}

void denginescene_scriptingsync_python_pull(ScriptingSyncFlags flags, Entity* entity, const EntityObject* pyobject)
{
    for(size_t i = 0; i < 3; i++)
    {
        entity->transform.position[i] = pyobject->transform->position->_xyzw[i];
        entity->transform.rotation[i] = pyobject->transform->rotation->_xyzw[i];
        entity->transform.scale[i] = pyobject->transform->scale->_xyzw[i];
    }
}
