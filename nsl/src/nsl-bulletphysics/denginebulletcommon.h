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
void destroyworld();

/* camera to use for debugger */
void enable_debugdrawer(const Camera* camera);

/* thr maximum amount of time the physics 
 * engine should run and immediately 
 * product a step. defaults to 1/60 (ie.
 * in one second, it can step up to 1 time)
 *
 * remember this is frequency so more is innaccurate but faster processing
 * and lower is accurate but slower processing ie (1/60 will be faster 
 * than 1/120)
 */
void set_maxfrequency(const btScalar& ffq);

/* multiplied with time since last step to speed or
 * slow the simulation. default is 1.0
 *
 * 0> to <1 produces slow motion simulation.
 * 0 pauses the simulation
 * >1 speeds up simulation. if you use this make sure not
 * to exceed maxfrequency to avoid glitchig of simulation
 */
void set_timescale(const btScalar& ts);

void stepworld();

/* created a body using entity. additionally it modifies entity 
 * such that it can sync transforms automatically 
 * when the world is stepped*/
int create_rb(Entity* entity, ECSPhysicsColShape shape, ECSPhysicsColShapeConfigBox* cfg, btScalar mass, btRigidBody** bodyref);

/* add callback to call after a physics step.
 * use this to update visual transforms 
 */
void add_tickcb(btInternalTickCallback cb);

/* sync physics to entity. when you call create_rb, it does this automatically */
void phy2ent(const btTransform& transform, Entity* entity);

#endif

