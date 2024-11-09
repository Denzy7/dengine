#include <stdio.h>
#include <stdlib.h> //malloc
#include <string.h> //memset

#include "dengine-scene/scene.h"
#include "dengine-utils/logging.h"
#include "dengine-utils/debug.h"
#include "dengine-utils/timer.h"
#include "dengine-utils/macros.h"

#include "dengine/draw.h"
#include "dengine/loadgl.h" //getFBO
#include "dengine/entrygl.h"

#include "dengine/viewport.h" //get_view
#include "dengine-script/script.h"
#ifdef DENGINE_SCRIPTING_PYTHON
#include "dengine-script/python/python.h"
#include "dengine-scene/scriptingsync/python/entity.h"
#endif

typedef struct
{
    vtor entities;
    MaterialID materialid;
    uint32_t abid;
    uint32_t ibid;
}BatchInfo;

typedef struct
{
    Entity* e;
    float d;
    size_t i;
}TrasnparencyInfo;

typedef void(*QueueFunc)(Scene* scene, SceneQueue* queue);
typedef struct
{
    SceneQueueType type;
    QueueFunc func;    
}QueueFuncFor;

void _denginescene_queuefunc_default(Scene* s, SceneQueue* q);
void _denginescene_queuefunc_finish(Scene* s, SceneQueue* q);
void _denginescene_queuefunc_camera(Scene* s, SceneQueue* q);


void _denginescene_queuefunc_light_transform_dir(Scene* s, SceneQueue* q);
void _denginescene_queuefunc_light_transform_point(Scene* s, SceneQueue* q);
void _denginescene_queuefunc_light_transform_spot(Scene* s, SceneQueue* q);

void _denginescene_queuefunc_mesh_shadow_dir(Scene* s, SceneQueue* q);
void _denginescene_queuefunc_mesh_shadow_point(Scene* s, SceneQueue* q);
void _denginescene_queuefunc_mesh_shadow_spot(Scene* s, SceneQueue* q);
void _denginescene_mesh_shadow_point_draw(Scene* s, PointLight* pL); /* reuse for spot light too */

void _denginescene_queuefunc_light(Scene* s, SceneQueue* q);

void _denginescene_queuefunc_mesh_opaque(Scene* s, SceneQueue* q);
void _denginescene_queuefunc_mesh_transparent(Scene* s, SceneQueue* q);
void _denginescene_drawskybox(Scene* s);

void _denginescene_queuefunc_run(Scene* s, SceneQueueType type);
static const QueueFuncFor queue_funcs[] = 
{
    {DENGINESCENE_QUEUE_CAMERA, _denginescene_queuefunc_camera},
    {DENGINESCENE_QUEUE_LIGHT_DIR, _denginescene_queuefunc_light},
    {DENGINESCENE_QUEUE_LIGHT_POINT, _denginescene_queuefunc_light},
    {DENGINESCENE_QUEUE_LIGHT_SPOT, _denginescene_queuefunc_light},
    {DENGINESCENE_QUEUE_MESH_OPAQUE, _denginescene_queuefunc_mesh_opaque},
    {DENGINESCENE_QUEUE_MESH_TRANSPARENT, _denginescene_queuefunc_mesh_transparent},
};

int denginescene_new(Scene** scene)
{
    DENGINE_DEBUG_ENTER;
    if((*scene = calloc(1, sizeof (Scene))) == NULL)
    {
        perror("calloc");
        return 0;
    }
#ifdef DENGINE_SCRIPTING_PYTHON
    if(!denginescript_isinit()){
        dengineutils_logging_log("ERROR::Scripting has to init to create scene when using Python");
        return 0;
    }
    ;

    if(!denginescript_python_module_scenemodule_new_entityobject(&((*scene)->entityobj))){
        dengineutils_logging_log("ERROR::Cannot create Python EntityObject we might be low on memory");
        return 0;
    }
#endif
    for(size_t i = 0; i < DENGINESCENE_QUEUE_COUNT; i++)
    {
        vtor_create_ptrs(&(*scene)->queues[i].entities);
        QueueFunc qf = _denginescene_queuefunc_default; 
        for(size_t j = 0; j < DENGINE_ARY_SZ(queue_funcs); j++)
        {
            if(queue_funcs[j].type == i)
                qf = queue_funcs[j].func;
        }
        (*scene)->queues[i].queue_func = qf;
        (*scene)->queues[i].type = i;
    }
    vtor_create_ptrs(&(*scene)->entities);

    (*scene)->queuerstate = DENGINESCENE_QUEUER_STATE_RUNNING;



    return 1;
}

int denginescene_new_skybox(const Primitive* cube, const Material* material, Skybox** skybox)
{
    DENGINE_DEBUG_ENTER;
    if((*skybox = calloc(1, sizeof(Skybox))) == NULL)
    {
        perror("calloc");
        return 0;
    }

    memcpy(&(*skybox)->cube, cube, sizeof(Primitive));
    memcpy(&(*skybox)->material, material, sizeof(Material));

    return 1;
}

void denginescene_destroy(Scene* scene)
{
    DENGINE_DEBUG_ENTER;

    for (uint32_t i = 0; i < scene->entities.count; i++) {
        Entity** root = scene->entities.data;
        denginescene_ecs_destroy_entity(root[i]);
    }

    for(size_t i = 0; i < DENGINESCENE_QUEUE_COUNT; i++)
    {
        vtor_free(&scene->queues[i].entities);
    }
    for(size_t i = 0; i < scene->batchinfo.count; i++)
    {
        BatchInfo* bis = scene->batchinfo.data;
        vtor_free(&bis[i].entities);
    }
    vtor_free(&scene->batchinfo);
    vtor_free(&scene->entities);

    if(scene->skybox)
        free(scene->skybox);

    free(scene);
}

void denginescene_add_entity(Scene* scene, Entity* entity)
{
    DENGINE_DEBUG_ENTER;
    vtor_pushback(&scene->entities, entity);
    if(scene->queuerstate == DENGINESCENE_QUEUER_STATE_RUNNING)
        denginescene_queuer_queue(scene);
}

void denginescene_update(Scene* scene)
{
    DENGINE_DEBUG_ENTER;

    Framebuffer entryfb; /* usually 0 if native or n if embedding in sth like gtk */
    dengine_entrygl_framebuffer(GL_FRAMEBUFFER, &entryfb);

    for (uint32_t i = 0; i < scene->entities.count; i++) {
        Entity** root = scene->entities.data;
        denginescene_ecs_transform_entity(root[i]);
    }

    _denginescene_queuefunc_light_transform_dir(scene, &scene->queues[DENGINESCENE_QUEUE_LIGHT_DIR]);
    _denginescene_queuefunc_light_transform_point(scene, &scene->queues[DENGINESCENE_QUEUE_LIGHT_POINT]);
    _denginescene_queuefunc_light_transform_spot(scene, &scene->queues[DENGINESCENE_QUEUE_LIGHT_SPOT]);

    _denginescene_queuefunc_mesh_shadow_dir(scene, &scene->queues[DENGINESCENE_QUEUE_LIGHT_DIR]);
    _denginescene_queuefunc_mesh_shadow_point(scene, &scene->queues[DENGINESCENE_QUEUE_LIGHT_POINT]);
    _denginescene_queuefunc_mesh_shadow_spot(scene, &scene->queues[DENGINESCENE_QUEUE_LIGHT_SPOT]);

    _denginescene_queuefunc_run(scene, DENGINESCENE_QUEUE_CAMERA); 

    dengine_framebuffer_bind(GL_FRAMEBUFFER, &entryfb);


    denginescene_scripts_run(scene, DENGINE_SCRIPT_FUNC_UPDATE);
}

void denginescene_queuer_state(Scene* scene, SceneQueuerState state)
{
    scene->queuerstate = state;
    if(state == DENGINESCENE_QUEUER_STATE_RUNNING)
        denginescene_queuer_queue(scene);
}

/* check if an entity and its children has certail components */
/* traverse scene tree to find components */
void _denginescene_traverse_entity(Entity* entity, Scene* scene)
{
    if(entity->camera_component)
        vtor_pushback(&scene->queues[DENGINESCENE_QUEUE_CAMERA].entities, entity);
    if(entity->light_component){
        if(entity->light_component->type == DENGINE_LIGHT_DIR)
            vtor_pushback(&scene->queues[DENGINESCENE_QUEUE_LIGHT_DIR].entities, entity);
        else if(entity->light_component->type == DENGINE_LIGHT_POINT)
            vtor_pushback(&scene->queues[DENGINESCENE_QUEUE_LIGHT_POINT].entities, entity);
        else if(entity->light_component->type == DENGINE_LIGHT_SPOT)
            vtor_pushback(&scene->queues[DENGINESCENE_QUEUE_LIGHT_SPOT].entities, entity);
    }
    if(entity->mesh_component)
        vtor_pushback(&scene->queues[DENGINESCENE_QUEUE_MESH].entities, entity);

    if(entity->scripts.count){
        vtor_pushback(&scene->queues[DENGINESCENE_QUEUE_SCRIPT].entities, entity);
    }

    for(size_t i = 0; i < entity->children.count; i++)
    {
        Entity* child = ((Entity**)entity->children.data)[i];
        _denginescene_traverse_entity(child, scene);
    }
}

int _isbatchable(BatchInfo* batchinfo, MeshComponent* mesh)
{
    /*TODO: add mre checks like vao */
    if(
            batchinfo->materialid == mesh->material.id &&
            batchinfo->abid == mesh->mesh.array.buffer_id &&
            batchinfo->ibid == mesh->mesh.index.buffer_id)
        return 1;

    return 0;
}

void denginescene_queuer_queue(Scene* scene)
{
    double t1, t2;

    dengineutils_timer_get_current_r(&t1);

    for(size_t i = 0; i < DENGINESCENE_QUEUE_COUNT; i++)
    {
        SceneQueue* q = &scene->queues[i];
        vtor_free(&q->entities);
        vtor_create_ptrs(&q->entities);
    }
    BatchInfo* bis = scene->batchinfo.data;
    for(size_t i = 0; i < scene->batchinfo.count; i++)
    {
        vtor_free(&bis[i].entities);
    }
    vtor_free(&scene->batchinfo);
    vtor_create(&scene->batchinfo, sizeof(BatchInfo));

    for(size_t i = 0; i < scene->entities.count; i++)
    {
        Entity* root = ((Entity**)scene->entities.data)[i];
        _denginescene_traverse_entity(root, scene);
    }

    for(size_t i = 0; i < scene->queues[DENGINESCENE_QUEUE_MESH].entities.count; i++)
    {

        Entity* e = ((Entity**)scene->queues[DENGINESCENE_QUEUE_MESH].entities.data)[i];
        if(e->mesh_component->material.shader_color.hint == DENGINE_SHADER_SCENEQUEUER_SHADERHINT_OPAQUE)
        {
            /* batch all opaque meshes */
            /* pushback the same materials together for batching */
            BatchInfo* batchinfos, *findbatchinfo = NULL, newbatchinfo;
            for(size_t j = 0; j < scene->batchinfo.count; j++)
            {
                batchinfos = scene->batchinfo.data;
                if(_isbatchable(&batchinfos[j], e->mesh_component))
                {
                    findbatchinfo = &batchinfos[j];
                    break;
                }
            }
            if(findbatchinfo)
            {
                vtor_pushback(&findbatchinfo->entities, e);
            }else {
                memset(&newbatchinfo, 0, sizeof(newbatchinfo));
                vtor_create_ptrs(&newbatchinfo.entities);
                vtor_pushback(&newbatchinfo.entities, e);
                newbatchinfo.abid = e->mesh_component->mesh.array.buffer_id;
                newbatchinfo.ibid = e->mesh_component->mesh.index.buffer_id;
                newbatchinfo.materialid = e->mesh_component->material.id;
                vtor_pushback(&scene->batchinfo, &newbatchinfo);
            }
        }else if(e->mesh_component->material.shader_color.hint == DENGINE_SHADER_SCENEQUEUER_SHADERHINT_TRANSPARENT)
        {
            vtor_pushback(&scene->queues[DENGINESCENE_QUEUE_MESH_TRANSPARENT].entities, e);
        }
    }
    dengineutils_timer_get_current_r(&t2);
    /*denginescene_queuer_dumpstats_stdio(scene, stdout);*/

    /* POST QUEUEING OPERATIONS */

    /* TODO: set shadow maps and light count. this work should probably be done
     * by the lighting system (lighting.c) so it can also  work with a lighting buffer object*/
    for(size_t i = 0; i < scene->queues[DENGINESCENE_QUEUE_MESH].entities.count; i++)
    {
        Entity* mesh = ((Entity**)scene->queues[DENGINESCENE_QUEUE_MESH].entities.data)[i];
        if(!mesh->mesh_component->material.shader_color.program_id)
            continue;

        dengine_shader_set_int(&mesh->mesh_component->material.shader_color, 
                "nr_pointLights", 
                scene->queues[DENGINESCENE_QUEUE_LIGHT_POINT].entities.count);
        dengine_shader_set_int(&mesh->mesh_component->material.shader_color, 
                "nr_spotLights", 
                scene->queues[DENGINESCENE_QUEUE_LIGHT_SPOT].entities.count);

        for(size_t j = 0; j < scene->queues[DENGINESCENE_QUEUE_LIGHT_DIR].entities.count; j++)
        {
            /* well just set for the first dirLight :))))) */
            if(j > 0)
                break;

            dengine_material_set_texture(
                    &((DirLight*)((Entity**)scene->queues[DENGINESCENE_QUEUE_LIGHT_DIR].entities.data)[j]->light_component->light)->shadow.shadow_map.depth,
                    "dLightShadow",
                    &mesh->mesh_component->material);

        }

        for(size_t j = 0; j < scene->queues[DENGINESCENE_QUEUE_LIGHT_POINT].entities.count; j++)
        {
            char idstr[128];
            PointLight* pl = ((Entity**)scene->queues[DENGINESCENE_QUEUE_LIGHT_POINT].entities.data)[j]->light_component->light;
            snprintf(idstr, sizeof(idstr), "pLightsShadow%u", (uint32_t)j);
            dengine_material_set_texture(
                    &pl->shadow.shadow_map.depth,
                    idstr,
                    &mesh->mesh_component->material);
        }
        for(size_t j = 0; j < scene->queues[DENGINESCENE_QUEUE_LIGHT_SPOT].entities.count; j++)
        {
            char idstr[128];
            SpotLight* sl = ((Entity**)scene->queues[DENGINESCENE_QUEUE_LIGHT_SPOT].entities.data)[j]->light_component->light;
            PointLight* pl = &sl->pointLight;
            snprintf(idstr, sizeof(idstr), "sLightsShadow%u", (uint32_t)j);
            dengine_material_set_texture(
                    &pl->shadow.shadow_map.depth,
                    idstr,
                    &mesh->mesh_component->material);
        }
    }

    dengineutils_logging_log("WARNING::Queuer took %.3f ms", (t2 - t1) * 1000.0);
}

void _denginescene_queuefunc_default(Scene* s, SceneQueue* q)
{
    /* no-op */
}

void _denginescene_drawentityaxis_recursive(Primitive* prim, const Shader* dft, Entity* e, float scale)
{
    for(uint32_t l = 0; l < 3; l++)
    {
        float color[3];
        mat4 mdl;
        vec3 scl;
        vec4 pos;
        glm_mat4_identity(mdl);
        glm_vec4_copy(e->transform.world_model[3], pos);
        glm_translate(mdl, pos);
        glm_vec3_fill(scl, scale);
        glm_rotate_x(mdl, glm_rad(e->transform.rotation[0]), mdl);
        glm_rotate_y(mdl, glm_rad(e->transform.rotation[1]), mdl);
        glm_rotate_z(mdl, glm_rad(e->transform.rotation[2]), mdl);
        glm_scale(mdl, scl);
        color[0] = l == 0 ? 1.0f : 0.0f, color[1] = l == 1 ? 1.0f : 0.0f, color[2] = l == 2 ? 1.0f : 0.0f;
        prim->offset = (void*)(l*2*sizeof (uint16_t));
        dengine_shader_set_mat4(dft, "model", mdl[0]);
        dengine_shader_set_vec3(dft, "color", color);
        dengine_draw_primitive(prim, dft);
    }
    for(size_t i = 0; i < e->children.count; i++)
    {
        Entity** ec = e->children.data;
        _denginescene_drawentityaxis_recursive(prim, dft, ec[i], scale);
    }
}

void _denginescene_queuefunc_camera(Scene* scene, SceneQueue* q)
{
    /* 
     * this whole setup works for 
     * multicamera coz of last cam.
     *
     * a camera should essentially trigger the entire render process
     * from lighting up to the skybox
     */
    DENGINE_DEBUG_ENTER;
    Entity** cameras = q->entities.data;
    for(size_t i = 0; i < q->entities.count; i++){
        Entity* camera = cameras[i];
        denginescene_ecs_do_camera_scene(camera, scene);
    }
}

void _denginescene_queuefunc_light(Scene* s, SceneQueue* q)
{
    Entity** lights = q->entities.data;
    for(size_t i = 0; i < q->entities.count; i++)
    {
        Entity* light = lights[i];
        Entity** meshes = s->queues[DENGINESCENE_QUEUE_MESH].entities.data; 
        for(size_t j = 0; j < s->queues[DENGINESCENE_QUEUE_MESH].entities.count; j++)
        {
            Entity* mesh = meshes[j];
            dengine_lighting_light_apply(light->light_component->type,
                    light->light_component->light,
                    &mesh->mesh_component->material.shader_color);
        }
    }
}

void _denginescene_queuefunc_light_transform_dir(Scene* s, SceneQueue* q)
{
    for(size_t i = 0; i < q->entities.count; i++)
    {
        Entity* light =  ((Entity**)q->entities.data)[i];
        DirLight* d = light->light_component->light;
        denginescene_ecs_get_front(light, d->direction);
    }
}

void _denginescene_queuefunc_light_transform_point(Scene* s, SceneQueue* q)
{
    for(size_t i = 0; i < q->entities.count; i++)
    {
        Entity* light =  ((Entity**)q->entities.data)[i];
        PointLight* p = light->light_component->light;
        glm_vec3_copy(light->transform.world_model[3], p->position);;
    }
}

void _denginescene_queuefunc_light_transform_spot(Scene* s, SceneQueue* q)
{
    for(size_t i = 0; i < q->entities.count; i++)
    {
        Entity* light =  ((Entity**)q->entities.data)[i];
        SpotLight* s = light->light_component->light;
        glm_vec3_copy(light->transform.world_model[3], s->pointLight.position);;
        denginescene_ecs_get_front(light, s->direction);
    }
}

void _denginescene_queuefunc_mesh_opaque(Scene* scene, SceneQueue* q)
{
    glDepthFunc(GL_LESS); DENGINE_CHECKGL;
    /* TODO: we probbly wanna get a hint from shader/material */
    glEnable(GL_CULL_FACE); DENGINE_CHECKGL;
    glCullFace(GL_BACK); DENGINE_CHECKGL;
    BatchInfo* batchinfo = scene->batchinfo.data ;
    for(size_t j = 0; j < scene->batchinfo.count; j++)
    {
        Entity** meshes = batchinfo[j].entities.data;
        /* use mesh 0 for initialization */
        Entity* mesh = meshes[0];
        dengine_material_use(&mesh->mesh_component->material);
        /* works with multicam since camera triggers mesh to skybox queue */
        dengine_camera_apply(&mesh->mesh_component->material.shader_color,
                &scene->last_cam->camera_component->camera);
        dengine_draw_sequence_start(&mesh->mesh_component->mesh, &mesh->mesh_component->material.shader_color);
        for(size_t k = 0; k < batchinfo[j].entities.count; k++)
        {
            mesh = meshes[k];
            if(mesh->mesh_component->drawmode != DENGINESCENE_ECS_MESHCOMPONENT_DRAWMODE_ENABLED)
                continue;
            dengine_shader_current_set_mat4(
                    "model",
                    mesh->transform.world_model[0]
                    );
            dengine_draw_sequence_draw();      
        }
        /*dengine_draw_sequence_end();*/
    }
}

/*TODO: rewrite our qsort to stop using retarted global variables */
Entity* _lastcam_global = NULL;
int _distcmp(const void* a, const void* b)
{
    /*cast to ignore meh compiler warning */
    Entity* ea = (void*)a;
    Entity* eb = (void*)b;
    vec3 pa, pb, pc;

    glm_vec3_copy(_lastcam_global->transform.world_model[3], pc);;
    glm_vec3_copy(ea->transform.world_model[3], pa);;
    glm_vec3_copy(eb->transform.world_model[3], pb);;
    float da = glm_vec3_distance(pc, pa);
    float db = glm_vec3_distance(pc, pb);
    if(da > db)
        return -1;
    else if(da < db)
        return 1;
    else 
        return 0;
}
void _denginescene_queuefunc_mesh_transparent(Scene* scene, SceneQueue* q)
{
    glDisable(GL_CULL_FACE); DENGINE_CHECKGL;
    glEnable(GL_BLEND); DENGINE_CHECKGL;
    
    glDepthFunc(GL_LESS); DENGINE_CHECKGL;
    /*TODO: we wanna get hint from material/shader */
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); DENGINE_CHECKGL;
    Entity** e = q->entities.data;
    _lastcam_global = scene->last_cam;
    qsort(e, q->entities.count, sizeof(Entity*), _distcmp);
    for(size_t i = 0; i < q->entities.count; i++)
    {
        Entity* mesh = e[i];
        dengine_material_use(&mesh->mesh_component->material);
/* works with multicam since camera triggers mesh to skybox queue */
        dengine_camera_apply(&mesh->mesh_component->material.shader_color,
                &scene->last_cam->camera_component->camera);
        dengine_draw_sequence_start(&mesh->mesh_component->mesh, &mesh->mesh_component->material.shader_color);
        dengine_shader_current_set_mat4(
                    "model",
                    mesh->transform.world_model[0]);
        dengine_draw_sequence_draw(); 
    }
}

void _denginescene_drawskybox(Scene* scene)
{
    if(scene->skybox == NULL)
        return;
    Camera* camera = &scene->last_cam->camera_component->camera;;
    // draw as the last entity
    
    glEnable(GL_CULL_FACE); DENGINE_CHECKGL;
    glCullFace(GL_FRONT); DENGINE_CHECKGL;
    glDepthFunc(GL_LEQUAL); DENGINE_CHECKGL;

    dengine_camera_apply(&scene->skybox->material.shader_color, camera);
    dengine_material_use(&scene->skybox->material);

    dengine_draw_primitive(&scene->skybox->cube, &scene->skybox->material.shader_color);
}

void _denginescene_queuefunc_run(Scene* s, SceneQueueType type)
{
    ((QueueFunc)s->queues[type].queue_func)(s, &s->queues[type]);
}

void _denginescene_queuefunc_mesh_shadow_dir(Scene* scene, SceneQueue* q)
{
    /* TODO: Cull front faces for shadows. Make this optional */
    glCullFace(GL_FRONT);
    DENGINE_CHECKGL;

    for(size_t i = 0; i < q->entities.count; i++)
    {
        Entity* light = ((Entity**)q->entities.data)[i];
        DirLight* dl = light->light_component->light;
        ShadowOp* shadowop = &dl->shadow;
        if(!shadowop->shadow_map.depth.texture_id)
            continue;
        /* we'd use shadowop_clear but it's repetitive with entrygl */
        dengine_framebuffer_bind(GL_FRAMEBUFFER, &shadowop->shadow_map);
        glClear(GL_DEPTH_BUFFER_BIT); DENGINE_CHECKGL;
        Entity** meshes = scene->queues[DENGINESCENE_QUEUE_MESH].entities.data; 
        for(size_t j = 0; j < scene->queues[DENGINESCENE_QUEUE_MESH].entities.count; j++)
        {
            Entity* mesh = meshes[j];
            Shader* shader = &mesh->mesh_component->material.shader_shadow;

            if(!shader->program_id)
                continue;

            if(!mesh->mesh_component->drawmode)
                continue;

            dengine_lighting_shadow_dirlight_draw(dl, shader, 
                    &mesh->mesh_component->mesh,
                    mesh->transform.world_model[0]
                    );
        }
    }
}

void _denginescene_mesh_shadow_point_draw(Scene* scene, PointLight* pl)
{
    /* TODO: Cull front faces for shadows. Make this optional */
    glCullFace(GL_FRONT);
    DENGINE_CHECKGL;

    Entity** meshes = scene->queues[DENGINESCENE_QUEUE_MESH].entities.data; 
    for(size_t i = 0; i < scene->queues[DENGINESCENE_QUEUE_MESH].entities.count; i++)
    {
        Entity* mesh = meshes[i];
        Shader* shader = &mesh->mesh_component->material.shader_shadow3d;

        if(!shader->program_id)
            continue;

        dengine_lighting_shadow_pointlight_draw(pl, shader, 
                &mesh->mesh_component->mesh,
                mesh->transform.world_model[0]
                );
    }

}
void _denginescene_queuefunc_mesh_shadow_point(Scene* scene, SceneQueue* q)
{
    for(size_t i = 0; i < q->entities.count; i++)
    {
        Entity* light = ((Entity**)q->entities.data)[i];
        PointLight* pl = light->light_component->light;
        ShadowOp* shadowop = &pl->shadow;
        if(!shadowop->shadow_map.depth.texture_id)
            continue;
        /* we'd use shadowop_clear but it's repetitive with entrygl */
        dengine_framebuffer_bind(GL_FRAMEBUFFER, &shadowop->shadow_map);
        glClear(GL_DEPTH_BUFFER_BIT); DENGINE_CHECKGL;
        _denginescene_mesh_shadow_point_draw(scene, pl);
    }
}

void _denginescene_queuefunc_mesh_shadow_spot(Scene* scene, SceneQueue* q)
{
    for(size_t i = 0; i < q->entities.count; i++)
    {
        Entity* light = ((Entity**)q->entities.data)[i];
        SpotLight* sl = light->light_component->light;
        ShadowOp* shadowop = &sl->pointLight.shadow;
        if(!shadowop->shadow_map.depth.texture_id)
            continue;
        /* we'd use shadowop_clear but it's repetitive with entrygl */
        dengine_framebuffer_bind(GL_FRAMEBUFFER, &shadowop->shadow_map);
        glClear(GL_DEPTH_BUFFER_BIT); DENGINE_CHECKGL;
        _denginescene_mesh_shadow_point_draw(scene, &sl->pointLight);
    }
}

void denginescene_scripts_run(Scene* scene, ScriptFunc func)
{
    /* running all scripts in scene we pass the entity or entityobj if python */
    void* scriptsarg = NULL;
#ifdef DENGINE_SCRIPTING_PYTHON
    scriptsarg = scene->entityobj;
#endif
    for(size_t i = 0; i < scene->queues[DENGINESCENE_QUEUE_SCRIPT].entities.count; i++)
    {
        Entity* e = ((Entity**)scene->queues[DENGINESCENE_QUEUE_SCRIPT].entities.data)[i];
        denginescene_ecs_scripts_run(e, func, scriptsarg);
    }
}
void denginescene_dumphierachy_stdio(Scene* scene, FILE* stdiof)
{
    for(size_t i = 0; i < scene->entities.count; i++)
    {
        Entity* e = ((Entity**)scene->entities.data)[i];
        denginescene_ecs_dumphierachy_stdio(e, stdiof);
    }
}

void denginescene_set_debugaxis(Scene* scene, int state)
{
    if(state)
    {
        /*TODO: destroy on scene_destroy */
        dengine_shader_make_standard(DENGINE_SHADER_DEFAULT, &scene->__dftshdr);
        dengine_primitive_gen_axis(&scene->__axis, &scene->__dftshdr);
        scene->__axis.index_count = 2;
    }
    scene->__axisdraw = state;
}

void denginescene_set_debuggrid(Scene* scene, int state)
{
    if(state)
    {
        /*TODO: destroy on scene_destroy */
        int slice = 10;
        dengine_shader_make_standard(DENGINE_SHADER_DEFAULT, &scene->__dftshdr);
        dengine_primitive_gen_grid(slice, &scene->__grid_a, &scene->__dftshdr);
        dengine_primitive_gen_grid(slice * 2, &scene->__grid_b, &scene->__dftshdr);
    }
    scene->__griddraw = state;
}

void denginescene_ecs_do_camera_scene(Entity* camera, Scene* scene)
{
    CameraComponent* camc = camera->camera_component;
    Camera* cam = &camera->camera_component->camera;
    //apply position
    glm_vec3_copy(camera->transform.world_model[3], cam->position);
    vec3 front;
    denginescene_ecs_get_front(camera, front);

    //store entry viewport
    int *x = &camc->entry_x,
        *y = &camc->entry_y,
        *w = &camc->entry_w,
        *h = &camc->entry_h;
    dengine_viewport_get(x, y, w, h);

    /* we'll be changing depth, blend and cull */
    camc->entry_depthon = glIsEnabled(GL_DEPTH_TEST); DENGINE_CHECKGL;
    camc->entry_cullon = glIsEnabled(GL_CULL_FACE); DENGINE_CHECKGL;
    camc->entry_blendon = glIsEnabled(GL_BLEND); DENGINE_CHECKGL;

    glGetIntegerv(GL_DEPTH_FUNC, &camc->entry_depthfunc);
    DENGINE_CHECKGL;
    glGetIntegerv(GL_CULL_FACE_MODE, &camc->entry_cullmode);
    DENGINE_CHECKGL;
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &camc->entry_srcalpha); DENGINE_CHECKGL;
    glGetIntegerv(GL_BLEND_DST_ALPHA, &camc->entry_dstalpha); DENGINE_CHECKGL;

    glEnable(GL_DEPTH_TEST); DENGINE_CHECKGL;

    dengine_viewport_set(0, 0,
            cam->render_width,cam->render_height);
    dengine_camera_lookat(front, cam);
    dengine_camera_project_perspective((float)cam->render_width / (float)cam->render_height,
            &camera->camera_component->camera);
    dengine_camera_use(cam);
    scene->last_cam = camera;

    _denginescene_drawskybox(scene);

    for(size_t k = DENGINESCENE_QUEUE_LIGHT_DIR; k <= DENGINESCENE_QUEUE_MESH_TRANSPARENT; k++)
    {
        SceneQueue* q = &scene->queues[k]; 
        QueueFunc qf = q->queue_func;
        qf(scene, q);
    }

    /*TODO: do this only for debug builds? */
#if 1
    if(scene->__griddraw || scene->__axisdraw)
        dengine_camera_apply(&scene->__dftshdr, cam);
    if(scene->__griddraw)
    {
        mat4 m;
        vec3 s;
        float d = glm_vec3_distance(camera->transform.position, GLM_VEC3_ZERO);
        d *= ((cam->far / 2.0f) / d);
        glm_vec3_fill(s, d);
        glm_mat4_identity(m);
        glm_scale(m, s);
        vec3 gridcol = {1.0f, 1.0f, 0.0f};
        dengine_shader_set_vec3(&scene->__dftshdr, "color", gridcol);
        dengine_shader_set_mat4(&scene->__dftshdr, "model", m[0]);
        glLineWidth(2.0f);
        dengine_draw_primitive(&scene->__grid_a, &scene->__dftshdr);
        glLineWidth(1.25f);
        dengine_draw_primitive(&scene->__grid_b, &scene->__dftshdr);
    }
    glDepthFunc(GL_ALWAYS);
    glLineWidth(3.0f);
    if(scene->__axisdraw)
    {
        for(size_t j = 0; j < scene->entities.count; j++)
        {
            Entity** e = scene->entities.data;
            _denginescene_drawentityaxis_recursive(&scene->__axis, &scene->__dftshdr, e[j], 0.75f);
        }
        /*            for(size_t k = 0; k < scene->queues[DENGINESCENE_QUEUE_MESH].entities.count; k++)*/
        /*{*/
        /*Entity** m = scene->queues[DENGINESCENE_QUEUE_MESH].entities.data;*/
        /*}*/

        /*for(size_t k = 0; k < scene->queues[DENGINESCENE_QUEUE_LIGHT].entities.count; k++)*/
        /*{*/
        /*Entity** m = scene->queues[DENGINESCENE_QUEUE_LIGHT].entities.data;*/
        /*_denginescene_drawentityaxis(&scene->__axis, &scene->__dftshdr, m[k], 0.5f);*/
        /*}*/
    }

#endif
    glBlendFunc(camc->entry_srcalpha, camc->entry_dstalpha); DENGINE_CHECKGL; 
    glDepthFunc(camc->entry_depthfunc); DENGINE_CHECKGL;
    glCullFace(camc->entry_cullmode); DENGINE_CHECKGL;
    if(!camc->entry_blendon){
        glDisable(GL_BLEND); DENGINE_CHECKGL;
    }
    if(!camc->entry_cullon){
        glDisable(GL_CULL_FACE); DENGINE_CHECKGL;
    }
    if(!camc->entry_depthon)
    {
        glDisable(GL_DEPTH_TEST); DENGINE_CHECKGL;
    }

    dengine_viewport_set(
            scene->last_cam->camera_component->entry_x,
            scene->last_cam->camera_component->entry_y,
            scene->last_cam->camera_component->entry_w,
            scene->last_cam->camera_component->entry_h);
}

void denginescene_queuer_dumpstats_stdio(Scene* scene, FILE* stdiof)
{
    /*dengineutils_logging_set_consolecolor(DENGINE_LOGGING_COLOR_YELLOW);*/
    for(size_t i = 0; i < DENGINESCENE_QUEUE_COUNT; i++)
    {
        fprintf(stdiof, "q=%u, entities : %u\n", (uint32_t)i, (uint32_t)scene->queues[i].entities.count);
    }
    fprintf(stdiof, "batches:%u\n", (uint32_t)scene->batchinfo.count);
    for(size_t i = 0; i < scene->batchinfo.count; i++)
    {
        BatchInfo* bi = &((BatchInfo*)scene->batchinfo.data)[i];
        fprintf(stdiof, "batch:%u n=%u, abid=%u, ibid=%u m=%u\n", 
                (uint32_t)i,
                (uint32_t)bi->entities.count,
                (uint32_t)bi->abid,
                (uint32_t)bi->ibid,
                (uint32_t)bi->materialid);
    }
    /*dengineutils_logging_set_consolecolor(DENGINE_LOGGING_COLOR_RESET);*/
}
