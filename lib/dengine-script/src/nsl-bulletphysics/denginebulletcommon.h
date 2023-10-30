#ifndef BULLET_PHYSICS_COMMON
#define BULLET_PHYSICS_COMMON
/* common interface using ECS */
#include <dengine/dengine.h>
#define BT_EULER_DEFAULT_ZYX
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
typedef enum
{
    DENGINE_ECS_PHYSICS_COLSHAPE_BOX,
    DENGINE_ECS_PHYSICS_COLSHAPE_CAPSULE,
}ECSPhysicsColShape;

typedef struct
{
    vec3 extends;
}ECSPhysicsColShapeConfigBox;

typedef struct
{
    Entity* ent;
    btRigidBody* body;
}ECSPhysicsBody;

/* set refworld to get reference to the world */
int initworld(btDynamicsWorld** refworld);
void startworld();
void destroyworld();

void stepworld();
void setontick(btInternalTickCallback tickcb);

int create_rb(Entity* entity, ECSPhysicsColShape shape, ECSPhysicsColShapeConfigBox* cfg, btScalar mass, btRigidBody** bodyref);

/* sync physics to entity */
void phy2ent(const btTransform& transform, Entity* entity);

#endif

