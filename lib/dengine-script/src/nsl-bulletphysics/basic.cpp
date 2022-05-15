#define BT_EULER_DEFAULT_ZYX
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <dengine/dengine.h>

btDynamicsWorld* world = NULL;
btAlignedObjectArray<btCollisionShape*> shapes;

void addBox(const btVector3& box_size, const btVector3& origin, const btVector3& rotation, const float& mass)
{
    btCollisionShape* box = new btBoxShape(box_size);
    shapes.push_back(box);

    btTransform trans;
    trans.setIdentity();
    trans.setOrigin(origin);

    btQuaternion rot_quat(rotation.getX(), rotation.getY(), rotation.getZ());
    trans.setRotation(rot_quat);

    //0 = static, else dynamic
    bool dynamic = mass != 0.0f;
    btVector3 intertia_l(0, 0, 0);
    btScalar mass_body(mass);
    if(dynamic)
        box->calculateLocalInertia(mass_body, intertia_l);

    //Track transforms
    btDefaultMotionState* state = new btDefaultMotionState(trans);
    btRigidBody::btRigidBodyConstructionInfo rb_info(mass_body,state, box, intertia_l);
    btRigidBody* body = new btRigidBody(rb_info);

    body->setSleepingThresholds(0,0);

    world->addRigidBody(body);
    printf("added box\n");
    //printf("Added a box. \nOrigin: x:%f, y:%f, z:%f\nPos:x:%f, y:%f, z:%f")
}

extern "C" int basic_start(Entity* entity)
{
    if(!world)
    {
        btDefaultCollisionConfiguration* config = new btDefaultCollisionConfiguration();
        btCollisionDispatcher* dispather = new btCollisionDispatcher(config);
        btBroadphaseInterface* broadphase = new btDbvtBroadphase();
        btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();

        world = new btDiscreteDynamicsWorld(dispather, broadphase, solver, config);
        world->setGravity(btVector3(0, -10, 0));
    }

    addBox(btVector3(5.0, 0.25, 5.0), btVector3(0, 0, 0),btVector3(0, 0, 0), 0.0);
    addBox(btVector3(1.0, 1.0, 1.0), btVector3(
                entity->transform.position[0],
                entity->transform.position[1],
                entity->transform.position[2]),
                btVector3(
                entity->transform.rotation[0],
                entity->transform.rotation[1],
                entity->transform.rotation[2]), 1.0);
    return 1;
}

extern "C" int basic_update(Entity* entity)
{
    world->stepSimulation(0.2f);
    btCollisionObject* obj = world->getCollisionObjectArray()[1];
    float model_mtx[16];
    obj->getWorldTransform().getOpenGLMatrix(model_mtx);
    // physics always in world space
    memcpy(&entity->transform.world_model[0][0], model_mtx, sizeof(model_mtx));
    return 1;
}
