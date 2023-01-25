#define BT_EULER_DEFAULT_ZYX
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <BulletDynamics/Vehicle/btRaycastVehicle.h>
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

extern "C" int car_create_rb(Entity* entity)
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
    }else if(entity->physics_component->shape == DENGINE_ECS_PHYSICS_COLSHAPE_CAPSULE)
    {
        shape = new btCapsuleShape(entity->transform.scale[0], entity->transform.scale[1]);
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
const btVector3 wheel_dir(0, -1, 0);
const btVector3 wheel_axle(-1, 0, 0);
const btScalar wheel_rest(0.6);
btDefaultVehicleRaycaster* caster;
btRaycastVehicle* vehicle;
btRaycastVehicle::btVehicleTuning tuning;
static int added = 0;
Entity* wheels[4];

extern "C" int car_setup_wheel(Entity* entity)
{
    btVector3 connection(entity->transform.position[0],
                         entity->transform.position[1] - 4.0,
                         entity->transform.position[2]);
    btWheelInfo& info = vehicle->addWheel(connection, wheel_dir, wheel_axle, wheel_rest, 1.5, tuning, added < 2);
    info.m_suspensionStiffness = 20.0f;
    info.m_wheelsDampingRelaxation = 2.3f;
    info.m_wheelsDampingCompression = 4.4f;
    info.m_frictionSlip = 10.f;
    info.m_rollInfluence = 0.02f;
    wheels[added] = entity;
    added++;
    return 1;
}

extern "C" int car_setup_chassis(Entity* entity)
{
    btRigidBody* carbody;


    btCollisionShape* box = new btBoxShape(btVector3(2.0,
                                                   0.5,
                                                   4.0));

    shapes.push_back(box);

    btTransform local;
    const float carmass = 800.0;
    local.setIdentity();
    local.setOrigin(btVector3(entity->transform.position[0],
                              entity->transform.position[1],
                              entity->transform.position[2]));

    btDefaultMotionState* state = new btDefaultMotionState(local);
    btVector3 inertia(0, 0, 0);
    box->calculateLocalInertia(carmass, inertia);

    btRigidBody::btRigidBodyConstructionInfo info(carmass, state, box, inertia);
    carbody = new btRigidBody(info);
    world->addRigidBody(carbody);

    caster = new btDefaultVehicleRaycaster(world);
    vehicle = new btRaycastVehicle(tuning, carbody, caster);

    carbody->setActivationState(DISABLE_DEACTIVATION);
    world->addVehicle(vehicle);
    vehicle->setCoordinateSystem(0, 1, 2);

    return 1;
}

extern "C" int car_world_start(ECSPhysicsWorld* wrld)
{
    config = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(config);
    broadphase = new btDbvtBroadphase();
    solver = new btSequentialImpulseConstraintSolver();

    world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, config);
    world->setGravity(btVector3(0, -9.8, 0));

    return 1;
}

extern "C" int car_update(Entity* entity)
{
    //btCollisionObject* obj = world->getCollisionObjectArray()[entity->physics_component->bodyid];
    float model_mtx[16];

    //dengineutils_logging_log("%f %f %f",
    //                         t.getOrigin().getX(),
    //                         t.getOrigin().getY(),
    //                         t.getOrigin().getZ());
    // physics always in world space
    vehicle->getChassisWorldTransform().getOpenGLMatrix(model_mtx);
    mat4 m;
    vec4 t;
    mat4 r;
    vec3 s;
    memcpy(&m[0][0], model_mtx, sizeof(mat4));
    glm_decompose(m, t, r, s);
    glm_vec4_copy3(t, entity->transform.position);
    glm_euler_angles(r, entity->transform.rotation);
    for(int j = 0; j < 3; j++){
        entity->transform.rotation[j] = glm_deg(entity->transform.rotation[j]);
    }
    static int dir_engine, dir_steer, dir_brake;
    if(dengine_input_get_key('W'))
        dir_engine = 1;
    else if(dengine_input_get_key('S'))
        dir_engine = -1;
    else
        dir_engine = 0;

    if(dengine_input_get_key('D'))
        dir_steer = -1;
    else if(dengine_input_get_key('A'))
        dir_steer = 1;
    else
        dir_steer = 0;

    if(dengine_input_get_key('X'))
        dir_brake = 1;
    else
        dir_brake = 0;

    if(dengine_input_get_key_once('R'))
    {
        btTransform transform;
        transform.setIdentity();

        transform.setOrigin(btVector3(0, 5, 0));
        transform.setRotation(btQuaternion(0, 0, 0));

        vehicle->getRigidBody()->setWorldTransform(transform);
        vehicle->getRigidBody()->getMotionState()->setWorldTransform(transform);

        vehicle->getRigidBody()->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
        vehicle->getRigidBody()->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
        vehicle->getRigidBody()->clearForces();
    }

    for(int i = 0; i < 4; i++)
    {
        if(i == 0 || i == 1){
            vehicle->setSteeringValue(0.5 * dir_steer, i);
        }else
        {
            vehicle->setBrake(200.0 * dir_brake, i);
        }

        vehicle->applyEngineForce(1750.0 * dir_engine, i);

        vehicle->getWheelTransformWS(i).getOpenGLMatrix(model_mtx);
        memcpy(&m[0][0], model_mtx, sizeof(model_mtx));
        glm_decompose(m, t, r, s);
        glm_vec4_copy3(t, wheels[i]->transform.position);
        glm_euler_angles(r, wheels[i]->transform.rotation);
        for(int j = 0; j < 3; j++){
            wheels[i]->transform.rotation[j] = glm_deg(wheels[i]->transform.rotation[j]);
        }

        //wheels[i]->transform.position[1] = vehicle->getWheelTransformWS(0).getOrigin().getY();
    }
    //printf("%f\n",vehicle->getWheelTransformWS(0).getOrigin().getY());

    return 1;
}

extern "C" int car_speed(void* arg)
{
    char speed[128];
    snprintf(speed, sizeof(speed), "Speed: %.1f Km/h", vehicle->getCurrentSpeedKmHour());
    int vp_x, vp_y;
    dengine_viewport_get(&vp_x, &vp_y, NULL, NULL);
    float fontsz = denginegui_get_fontsz();
    denginegui_text(fontsz, fontsz * 2, speed, NULL);
    return 1;
}
extern "C" int car_world_update(ECSPhysicsWorld* wld)
{
    static btScalar last = 0;
    btScalar now = dengineutils_timer_get_current() / 1000.0;
    world->stepSimulation(now - last, wld->substeps, wld->timestep_fixed);
    last = now;
    return 1;
}

extern "C" int car_world_terminate(void* args)
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

    delete vehicle;
    delete caster;

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
