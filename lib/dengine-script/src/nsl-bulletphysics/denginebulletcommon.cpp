#include "denginebulletcommon.h"
#include <BulletCollision/CollisionDispatch/btCollisionDispatcherMt.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolverMt.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorldMt.h>
#include <dengine-utils/thread.h>
#include <time.h>
#include <vector>

btScalar _fq = 1.0 / 60.0, _tscale = 1.0, _ffq = 18.0 / 60.0;
btDynamicsWorld* _world = NULL;
btDefaultCollisionConfiguration* config = NULL;
btCollisionDispatcher* dispatcher = NULL;
btBroadphaseInterface* broadphase = NULL;
btConstraintSolver* solver = NULL;
btAlignedObjectArray<btCollisionShape*> shapes;

std::vector<btInternalTickCallback> _tickcbs;
void _tickcb(btDynamicsWorld* world, btScalar timeStep)
{
    for(size_t i = 0; i < _tickcbs.size(); i++)
    {
        _tickcbs[i](world, timeStep);
    }
}

int initworld(btDynamicsWorld** refworld)
{
    btITaskScheduler* sched = btCreateDefaultTaskScheduler();
    broadphase = new btDbvtBroadphase();
    if(sched == NULL)
    {
        dengineutils_logging_log("WARNING::default scheduler not available. using single thread");
        config = new btDefaultCollisionConfiguration();
        dispatcher = new btCollisionDispatcher(config);
        solver = new btSequentialImpulseConstraintSolver();
        _world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, config);
    }else {
        btSetTaskScheduler(sched);
        dengineutils_logging_log("INFO::multithread sched %s threads=%d",
                sched->getName(),
                sched->getNumThreads()
                );
        btDefaultCollisionConstructionInfo config_info;
        config_info.m_defaultMaxPersistentManifoldPoolSize = 80000;
        config_info.m_defaultMaxCollisionAlgorithmPoolSize = 80000;
        config = new btDefaultCollisionConfiguration(config_info);
        dispatcher = new btCollisionDispatcherMt(config);
        btConstraintSolver* solvers[BT_MAX_THREAD_COUNT];
        for(uint64_t i = 0; i < BT_MAX_THREAD_COUNT; i++)
        {
            solvers[i] = new btSequentialImpulseConstraintSolver();
        }
        btConstraintSolverPoolMt* solver_pool = new btConstraintSolverPoolMt(solvers, BT_MAX_THREAD_COUNT);
        solver = new btSequentialImpulseConstraintSolverMt();
        _world = new btDiscreteDynamicsWorldMt(dispatcher, broadphase, solver_pool, solver, config);
    }
    _world->getSolverInfo().m_solverMode = 
        SOLVER_SIMD | SOLVER_USE_WARMSTARTING | 0;
    _world->getSolverInfo().m_numIterations = 10;

    _world->setGravity(btVector3(0, -9.8, 0));
    if(refworld)
        *refworld = _world;

   _world->setInternalTickCallback(_tickcb); 
    return 1;
}

void stepworld()
{
    _world->stepSimulation(_fq * _tscale, 0, _ffq);
}

void destroyworld()
{
    //remove the rigidbodies from the dynamics world and delete them
    for (int i = _world->getNumCollisionObjects() - 1; i >= 0; i--)
    {
        btCollisionObject* obj = _world->getCollisionObjectArray()[i];
        btRigidBody* body = btRigidBody::upcast(obj);
        if (body && body->getMotionState())
        {
            delete body->getMotionState();
        }
        _world->removeCollisionObject(obj);
        delete obj;
    }

    //delete collision shapes
    for (int j = 0; j < shapes.size(); j++)
    {
       btCollisionShape* shape = shapes[j];
       shapes[j] = 0;
       delete shape;
    }

    //delete dynamics world
    delete _world;

    //delete solver
    delete solver;

    //delete broadphase
    delete broadphase;

    //delete dispatcher
    delete dispatcher;

    delete config;

}
int create_rb(Entity* entity, ECSPhysicsColShape shape, ECSPhysicsColShapeConfigBox* cfg, btScalar mass, btRigidBody** bodyref)
{
    btCollisionShape* btshape = nullptr;
    if(shape == DENGINE_ECS_PHYSICS_COLSHAPE_BOX)
    {
        if(cfg != NULL)
        {
            btshape = new btBoxShape(btVector3(cfg->extends[0], cfg->extends[1], cfg->extends[2]));
        }else
        {
            btshape = new btBoxShape(btVector3(entity->transform.scale[0], entity->transform.scale[1], entity->transform.scale[2]));
        }
    }else if(shape == DENGINE_ECS_PHYSICS_COLSHAPE_CAPSULE)
    {
        btshape = new btCapsuleShape(entity->transform.scale[0], entity->transform.scale[1]);
    }else {
        dengineutils_logging_log("ERROR::no valid shape supplied");
        return 0;
    }

    shapes.push_back(btshape);
    btTransform trans;
    trans.setIdentity();
    trans.setOrigin(btVector3(
                entity->transform.position[0],
                entity->transform.position[1],
                entity->transform.position[2]));

    btScalar rx, ry, rz;
    rx = glm_rad(entity->transform.rotation[0]);
    ry = glm_rad(entity->transform.rotation[1]);
    rz = glm_rad(entity->transform.rotation[2]);
    btQuaternion rot_quat;
    rot_quat.setEulerZYX(rz, ry, rx);
    trans.setRotation(rot_quat);

    //0 = static, else dynamic
    bool dynamic = mass > 0.0f;
    btVector3 intertia_l(0, 0, 0);
    if(dynamic)
        btshape->calculateLocalInertia(mass, intertia_l);

    //Track transforms
    btDefaultMotionState* state = new btDefaultMotionState(trans);
    btRigidBody::btRigidBodyConstructionInfo rb_info(mass,state, btshape, intertia_l);
    btRigidBody* body = new btRigidBody(rb_info);

    body->setActivationState(DISABLE_DEACTIVATION);

    _world->addRigidBody(body);
    if(bodyref)
        *bodyref = body;
    //entity->physics_component->bodyid = world->getNumCollisionObjects() - 1;

    dengineutils_logging_log("INFO::Created rb for %u [%s] mass:%f", entity->entity_id, entity->name, mass);
    return 1;

}

void phy2ent(const btTransform& transform, Entity* entity)
{
    mat4 r;
    vec3 s;

    transform.getOpenGLMatrix((float*)entity->transform.world_model);
    glm_vec4_copy3(entity->transform.world_model[3], entity->transform.position);
    glm_decompose_rs(entity->transform.world_model, r, s);
    glm_euler_angles(r, entity->transform.rotation);
    for(int j = 0; j < 3; j++){
        entity->transform.rotation[j] = glm_deg(entity->transform.rotation[j]);
    }
}

void set_frequency(const btScalar& fq)
{
    _fq = fq;
}

void set_timescale(const btScalar& ts)
{
    _tscale = ts;
}

void add_tickcb(btInternalTickCallback cb)
{
    _tickcbs.push_back(cb);
}

void set_maxfrequency(const btScalar& ffq)
{
    _ffq = ffq;
}
