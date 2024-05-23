#include "denginebulletcommon.h"
#include <dengine-utils/thread.h>
#include <time.h>
btDynamicsWorld* _world = NULL;
btDefaultCollisionConfiguration* config = NULL;
btCollisionDispatcher* dispatcher = NULL;
btBroadphaseInterface* broadphase = NULL;
btSequentialImpulseConstraintSolver* solver = NULL;
btAlignedObjectArray<btCollisionShape*> shapes;

/*Thread physicsthread;*/
/*Condition physicsthread_start;*/
/*int physicsthr_run = 0, physicsthr_step = 0;*/

/*void* physicsthr(void* thr)*/
/*{*/
    /*dengineutils_thread_condition_wait(&physicsthread_start, &physicsthr_run); */
    /*static const btScalar fixed = 1.0f / 60.0f;*/

    /*double current = 0;*/
    /*double last = current;*/

    /*struct timespec spec;*/
    /*memset(&spec, 0, sizeof(spec));*/
    /*spec.tv_nsec = fixed * 1e9;*/
    /*while(physicsthr_run)*/
    /*{*/
        /*dengineutils_thread_condition_wait(&physicsthread_start, &physicsthr_step); */
        /*physicsthr_step = 0;*/
        /*dengineutils_timer_get_current_r(&current);*/
        /*current /= 1000.0;*/
        /*if(last == 0)*/
            /*last = current;*/
        /*_world->stepSimulation(current - last, 1, fixed);*/
        /*last = current; */
    /*}*/
    /*return NULL;*/
/*}*/

double _lastts = 0.0;
int initworld(btDynamicsWorld** refworld)
{
    config = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(config);
    broadphase = new btDbvtBroadphase();
    solver = new btSequentialImpulseConstraintSolver();

    _world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, config);
    _world->setGravity(btVector3(0, -9.8, 0));
/*    dengineutils_thread_condition_create(&physicsthread_start);*/
    /*dengineutils_thread_create(physicsthr, NULL, &physicsthread);*/
    /*dengineutils_thread_set_name(&physicsthread, "PhysicsThr");*/
    if(refworld)
        *refworld = _world;
    
    firststep();
    return 1;
}

void startworld()
{
    //dengineutils_thread_condition_raise(&physicsthread_start);
    //physicsthr_run = 1;
}

void firststep()
{
    dengineutils_timer_get_current_r(&_lastts);
    _lastts /= 1000.0;
}

void stepworld()
{
/*    if(!physicsthr_run)*/
        /*return;*/
    /*physicsthr_step = 1;*/
    /*dengineutils_thread_condition_raise(&physicsthread_start);*/

    double t;
    dengineutils_timer_get_current_r(&t);
    t /= 1000.0;
    _world->stepSimulation((t - _lastts) * (1.125), 0);
    _lastts = t;
}

void destroyworld()
{
/*    physicsthr_run = 0;*/
    /*stepworld();*/
    /*dengineutils_thread_wait(&physicsthread);*/
    /*dengineutils_thread_condition_destroy(&physicsthread_start);*/

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
