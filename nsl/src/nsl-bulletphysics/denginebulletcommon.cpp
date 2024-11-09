#include "denginebulletcommon.h"
#include <BulletCollision/CollisionDispatch/btCollisionDispatcherMt.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolverMt.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorldMt.h>
#include <dengine-utils/thread.h>
#include <time.h>
#include <vector>


btScalar _tscale = 1.0, _ffq = 1.0 / 60.0;
btDynamicsWorld* _world = NULL;
btDefaultCollisionConfiguration* config = NULL;
btCollisionDispatcher* dispatcher = NULL;
btBroadphaseInterface* broadphase = NULL;
btConstraintSolver* solver = NULL;
btAlignedObjectArray<btCollisionShape*> shapes;

ATTRIBUTE_ALIGNED16(class)
    DengineDebugDrawer: public btIDebugDraw
{
    DefaultColors m_ourColors;
    int m_debugMode;
    Primitive line;
    Shader dftshr;

    public:
    const Camera* debugcam;

    BT_DECLARE_ALIGNED_ALLOCATOR();
    DengineDebugDrawer()
    {
        /* require opengl to be loaded */
        if(glad_glGetError == NULL)
        {
            printf("we need opengl to be initialized before loading the nsl\n");
            exit(1);
        }
        m_debugMode = btIDebugDraw::DBG_DrawWireframe; 
        setDebugMode(m_debugMode);
        dengine_shader_make_standard(DENGINE_SHADER_DEFAULT, &dftshr);
        dengine_primitive_gen_line(&line, &dftshr);
        dengine_buffer_bind(GL_ARRAY_BUFFER, &line.array);
        /* well be chainging this every line point so we steam it */
        line.array.usage = GL_DYNAMIC_DRAW;
        glBufferData(GL_ARRAY_BUFFER, sizeof(btScalar) * 6, NULL, line.array.usage);
        DENGINE_CHECKGL;
    }

    virtual ~DengineDebugDrawer()
    {
    }

    virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
    {
        mat4 model;
        glm_mat4_identity(model);
        btScalar buf[6];
        memcpy(buf, to.m_floats, sizeof(btScalar) * 3);
        memcpy(buf + 3, from.m_floats, sizeof(btScalar) * 3);
            
        //glm_translate(model, pos);

        dengine_shader_set_vec3(&dftshr, "color", color.m_floats);
        dengine_shader_set_mat4(&dftshr, "model", &model[0][0]);
        dengine_camera_apply(&dftshr, debugcam);
        dengine_buffer_bind(GL_ARRAY_BUFFER, &line.array);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(btScalar) * 6, buf); DENGINE_CHECKGL;
        dengine_draw_primitive(&line, &dftshr);
    }

    virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
    {
        drawLine(PointOnB, PointOnB + normalOnB * distance, color);
        btVector3 ncolor(0, 0, 0);
        drawLine(PointOnB, PointOnB + normalOnB * 0.01, ncolor);
    }

    virtual void reportErrorWarning(const char* warningString)
    {
    }

    virtual void draw3dText(const btVector3& location, const char* textString)
    {
    }

    virtual void setDebugMode(int debugMode)
    {
        m_debugMode = debugMode;
    }

    virtual int getDebugMode() const
    {
        return m_debugMode;
    }
}_drawer;


std::vector<btInternalTickCallback> _tickcbs;
std::vector<ECSPhysicsBody> _epbs;
void _tickcb(btDynamicsWorld* world, btScalar timeStep)
{
    for(size_t i = 0; i < _epbs.size(); i++)
    {
        ECSPhysicsBody epb = _epbs.at(i);
        phy2ent(epb.body->getWorldTransform(), epb.ent);
    }
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

    _world->setDebugDrawer(&_drawer);

   _world->setInternalTickCallback(_tickcb); 
    return 1;
}

void stepworld()
{
    static double p = 0;;
    double t;
    dengineutils_timer_get_current_r(&t);
    t /= 1000.0f;
    if(p == 0)
        p = t;
    _world->stepSimulation((t - p) * _tscale , 0, _ffq);
    p = t;
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

    ECSPhysicsBody epb;
    epb.body = body;
    epb.ent = entity;
    _epbs.push_back(epb);

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

void enable_debugdrawer(const Camera* camera)
{
    _drawer.debugcam = camera;
}
