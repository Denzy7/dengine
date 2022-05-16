#define BT_EULER_DEFAULT_ZYX
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <dengine/dengine.h>

btDynamicsWorld* world = NULL;
btDefaultCollisionConfiguration* config = NULL;
btCollisionDispatcher* dispatcher = NULL;
btBroadphaseInterface* broadphase = NULL;
btSequentialImpulseConstraintSolver* solver = NULL;
btAlignedObjectArray<btCollisionShape*> shapes;

void applyForce(btRigidBody* body, const btVector3& force)
{
    body->applyForce(force, btVector3(0, 0, 0));
}

void applyTorque(btRigidBody* body, const btVector3& force)
{
    body->applyTorque(force);
}

extern "C" int basic_create_rb(Entity* entity)
{
    btCollisionShape* shape = nullptr;
    if(entity->physics_component->shape == DENGINE_ECS_PHYSICS_COLSHAPE_BOX)
    {
        ECSPhysicsColShapeConfigBox* cfg = (ECSPhysicsColShapeConfigBox*)entity->physics_component->colshapeconfig;
        if(cfg)
        {
            shape = new btBoxShape(btVector3(cfg->extends[0], cfg->extends[1], cfg->extends[2]));
        }else
        {
            shape = new btBoxShape(btVector3(entity->transform.scale[0], entity->transform.scale[1], entity->transform.scale[2]));
        }
    }

    if(shape)
    {
        shapes.push_back(shape);
        btTransform trans;
        trans.setIdentity();
        trans.setOrigin(btVector3(
                        entity->transform.position[0],
                        entity->transform.position[1],
                        entity->transform.position[2]));

        btQuaternion rot_quat(
                    entity->transform.rotation[0],
                    entity->transform.rotation[1],
                    entity->transform.rotation[2]);
        trans.setRotation(rot_quat);

        //0 = static, else dynamic
        bool dynamic = entity->physics_component->mass != 0.0f;
        btVector3 intertia_l(0, 0, 0);
        btScalar mass_body(entity->physics_component->mass);
        if(dynamic)
            shape->calculateLocalInertia(mass_body, intertia_l);

        //Track transforms
        btDefaultMotionState* state = new btDefaultMotionState(trans);
        btRigidBody::btRigidBodyConstructionInfo rb_info(mass_body,state, shape, intertia_l);
        btRigidBody* body = new btRigidBody(rb_info);

        body->setSleepingThresholds(0,0);

        world->addRigidBody(body);
        entity->physics_component->bodyid = world->getNumCollisionObjects() - 1;

        dengineutils_logging_log("INFO::Created rb for %u [%s]", entity->entity_id, entity->name);
        return 1;
    }else
    {
        dengineutils_logging_log("ERROR::no valid shape supplied");
        return 0;
    }
}

extern "C" int basic_world_start(void* args)
{
    config = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(config);
    broadphase = new btDbvtBroadphase();
    solver = new btSequentialImpulseConstraintSolver();

    world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, config);
    world->setGravity(btVector3(0, -10, 0));
    return 1;
}

extern "C" int basic_update(Entity* entity)
{
    btCollisionObject* obj = world->getCollisionObjectArray()[entity->physics_component->bodyid];
    float model_mtx[16];
    obj->getWorldTransform().getOpenGLMatrix(model_mtx);
    // physics always in world space
    memcpy(&entity->transform.world_model[0][0], model_mtx, sizeof(model_mtx));
    return 1;
}

extern "C" int basic_world_update(void* args)
{
    world->stepSimulation(1./60., 10);
    return 1;
}

extern "C" int basic_world_terminate(void* args)
{
    //remove the rigidbodies from the dynamics world and delete them
    for (int i = world->getNumCollisionObjects() - 1; i >= 0; i--)
    {
        btCollisionObject* obj = world->getCollisionObjectArray()[i];
        btRigidBody* body = btRigidBody::upcast(obj);
        if (body && body->getMotionState())
        {
            delete body->getMotionState();
        }
        world->removeCollisionObject(obj);
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
    delete world;

    //delete solver
    delete solver;

    //delete broadphase
    delete broadphase;

    //delete dispatcher
    delete dispatcher;

    delete config;

    return 1;
}

extern "C" int forces_update(Entity* entity)
{
    btCollisionObject* obj = world->getCollisionObjectArray()[entity->physics_component->bodyid];
    btRigidBody* body = btRigidBody::upcast(obj);

    static const float force = 150.0f;
    if(dengine_input_get_key('F'))
        applyForce(body, btVector3(0, force, 0));
    if(dengine_input_get_key('G'))
        applyForce(body, btVector3(0, -force, 0));

    if(dengine_input_get_key('W'))
        applyForce(body, btVector3(0, 0, -force));
    if(dengine_input_get_key('S'))
        applyForce(body, btVector3(0, 0, force));
    if(dengine_input_get_key('A'))
        applyForce(body, btVector3(-force, 0, 0));
    if(dengine_input_get_key('D'))
        applyForce(body, btVector3(force, 0, 0));

    if(dengine_input_get_key('Z'))
        applyTorque(body, btVector3(0, force, 0));
    if(dengine_input_get_key('X'))
        applyTorque(body, btVector3(0, -force, 0));

    if(dengine_input_get_key('E'))
        applyTorque(body, btVector3(0, 0,force));
    if(dengine_input_get_key('Q'))
        applyTorque(body, btVector3(0, 0, -force));

    if(dengine_input_get_key_once('R'))
    {
        btTransform transform;
        transform.setIdentity();

        transform.setOrigin(btVector3(0, 5, 0));
        transform.setRotation(btQuaternion(0, 0, 0));

        body->setWorldTransform(transform);
        body->getMotionState()->setWorldTransform(transform);

        body->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
        body->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
        body->clearForces();
    }

    return 1;
}
