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
 * product a step. defaults to 18/60 (ie.
 * in one second, it can step up to 18 times)
 * but should always be >= frequency 
 * to avoid glitching due to not physics
 * not running enough
 *
 * remember this is frequency so less is faster processing
 * and lower is slower processing ie (1/60 will be faster 
 * than 18/60)
 */
void set_maxfrequency(const btScalar& ffq);

/* simulation frequency. usually 1/60 (ie. 
 * in one second, it will produce 1 physics step),
 * high frequency may produce better result 
 * but use alot of resource 
 *
 * do not exceed the maxfrequency or simlation
 * starts glitching*/
void set_frequency(const btScalar& fq);

/* multiplied with frequency to speed or
 * slow the simulation. default is 1.0
 *
 * 0> to <1 produces slow motion simulation.
 * 0 pauses the simulation
 * >1 speeds up simulation. if you use this make sure not
 * to exceed maxfrequency to avoid glitchig of simulation
 * 
 * increasing timescale >1 implicitly increases the
 * frequency, so it uses more resources
 */
void set_timescale(const btScalar& ts);

void stepworld();

int create_rb(Entity* entity, ECSPhysicsColShape shape, ECSPhysicsColShapeConfigBox* cfg, btScalar mass, btRigidBody** bodyref);

/* add callback to call after a physics step.
 * use this to update visual transforms 
 */
void add_tickcb(btInternalTickCallback cb);

/* sync physics to entity */
void phy2ent(const btTransform& transform, Entity* entity);

#endif

