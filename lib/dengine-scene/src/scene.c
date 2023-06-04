#include <stdlib.h> //malloc
#include <string.h> //memset

#include "dengine-scene/scene.h"
#include "dengine-utils/logging.h"
#include "dengine-utils/debug.h"

#include "dengine/draw.h"
#include "dengine/loadgl.h" //getFBO
#include "dengine/entrygl.h"
#include "dengine_config.h"// DENGINE_ECS_MAXCHILDREN

#include "dengine/viewport.h" //get_view

#ifdef DENGINE_SCRIPTING_PYTHON
#include "dengine-script/py_modules/scenemodule.h" //create dummy scene entity PyObject* for push/pull
#endif
void _denginescene_do_check_camera(Entity* root, Scene* scene);

void _denginescene_do_check_light(Entity* root, Scene* scene);

void _denginescene_do_check_script(Entity* root, Scene* scene, ScriptFunc callfunc);

Scene* denginescene_new()
{
    DENGINE_DEBUG_ENTER;
    Scene* newscn = malloc(sizeof (struct _Scene));
    memset(newscn, 0, sizeof (Scene));

    vtor_create(&newscn->entities, sizeof(EntityChild));

#ifdef DENGINE_SCRIPTING_PYTHON
    if(denginescript_isinit())
        newscn->dummyentityobj = denginescript_pymod_scene_entity_new();
#endif

    return newscn;
}

Skybox*
denginescene_new_skybox(const Primitive* cube, const Material* material)
{
    DENGINE_DEBUG_ENTER;

    Skybox* sky = calloc(1, sizeof(Skybox));
    Primitive* c = calloc(1, sizeof(Primitive));
    Material* m = calloc(1, sizeof(Material));

    memcpy(c, cube, sizeof(Primitive));
    memcpy(m, material, sizeof(Material));

    sky->cube = c;
    sky->material = m;

    return sky;
}

void denginescene_destroy(Scene* scene)
{
    DENGINE_DEBUG_ENTER;

    EntityChild* ec = scene->entities.data;
    for (uint32_t i = 0; i < scene->entities.count; i++) {
        denginescene_ecs_destroy_entity(ec[i].child);
    }

    vtor_free(&scene->entities);

    if(scene->skybox)
    {
        free(scene->skybox->cube);
        free(scene->skybox->material);
        free(scene->skybox);
    }

    free(scene);
}

void denginescene_add_entity(Scene* scene, Entity* entity)
{
    DENGINE_DEBUG_ENTER;

    EntityChild ec = { entity };
    vtor_pushback(&scene->entities, &ec);
}

void denginescene_update(Scene* scene)
{
    DENGINE_DEBUG_ENTER;

    EntityChild* ec = scene->entities.data;
    for (uint32_t i = 0; i < scene->entities.count; i++) {
        Entity* root = ec[i].child;
        denginescene_ecs_transform_entity(root);
        _denginescene_do_check_camera(root,scene);
        _denginescene_do_check_light(root, scene);
        _denginescene_do_check_script(root, scene, DENGINE_SCRIPT_FUNC_UPDATE);
    }
}

void _denginescene_ecs_do_camera_draw_mesh(Entity* camera, Entity* mesh)
{
    //dengineutils_logging_log("drw %u",mesh->entity_id);
    dengine_material_use(mesh->mesh_component->material);
    dengine_camera_apply(&mesh->mesh_component->material->shader_color,camera->camera_component->camera);

    dengine_shader_set_mat4(&mesh->mesh_component->material->shader_color,
                            "model",
                            mesh->transform.world_model[0]
                            );

    dengine_draw_primitive(mesh->mesh_component->mesh,&mesh->mesh_component->material->shader_color);
}

void _denginescene_ecs_do_camera_draw(Entity* camera,Entity* root)
{
    if(!root->parent && root->mesh_component)
        _denginescene_ecs_do_camera_draw_mesh(camera,root);

    size_t children_count = root->children.count;
    EntityChild* ec = root->children.data;
    for (size_t i = 0; i < children_count; i++) {
        Entity* child = ec[i].child;
        if(child->mesh_component)
        {
            _denginescene_ecs_do_camera_draw_mesh(camera,child);
        }
        _denginescene_ecs_do_camera_draw(camera,child);
    }
}

void _denginescene_do_check_camera(Entity* root, Scene* scene)
{
    //check camera comp and draw
    if(!root->parent && root->camera_component)
    {
        denginescene_ecs_do_camera_scene(root,scene);
    }

    size_t children_count = root->children.count;
    EntityChild* ec = root->children.data;
    for (size_t i = 0; i < children_count; i++) {
        Entity* child = ec[i].child;
        if(child->camera_component)
        {
            denginescene_ecs_do_camera_scene(child,scene);
        }
        _denginescene_do_check_camera(child, scene);
    }
}

void _denginescene_do_check_light(Entity* root, Scene* scene)
{
    //check lightcomp comp and draw
    if(!root->parent && root->light_component)
    {
        denginescene_ecs_do_light_scene(root,scene);
    }

    size_t children_count = root->children.count;
    EntityChild* ec = root->children.data;
    for (size_t i = 0; i < children_count; i++) {
        Entity* child = ec[i].child;
        if(child->light_component)
        {
            denginescene_ecs_do_light_scene(child,scene);
        }
        _denginescene_do_check_light(child, scene);
    }
}

void _denginescene_do_check_script(Entity* root, Scene* scene, ScriptFunc callfunc)
{
    //check scriptcomp and exec callfunc
    if(!root->parent && root->scripts.count)
    {
        Script* scripts = root->scripts.data;
        for(size_t i = 0; i < root->scripts.count; i++)
        {
            if(scripts[i].type == DENGINE_SCRIPT_TYPE_PYTHON)
            {
                #ifdef DENGINE_SCRIPTING_PYTHON
                denginescene_ecs_do_script_entity(root, &scripts[i], callfunc , scene->dummyentityobj);
                #endif
            }else if(scripts[i].type == DENGINE_SCRIPT_TYPE_NSL)
            {
                 denginescene_ecs_do_script_entity(root, &scripts[i], callfunc, root);
            }
        }
    }

    size_t children_count = root->children.count;
    EntityChild* ec = root->children.data;
    for (size_t i = 0; i < children_count; i++) {
        Entity* child = ec[i].child;
        if(child->scripts.count)
        {
            Script* scripts = child->scripts.data;
            for(size_t i = 0; i < child->scripts.count; i++)
            {
                if(scripts[i].type == DENGINE_SCRIPT_TYPE_PYTHON)
                {
                    #ifdef DENGINE_SCRIPTING_PYTHON
                    denginescene_ecs_do_script_entity(child, &scripts[i], callfunc , scene->dummyentityobj);
                    #endif
                }else if(scripts[i].type == DENGINE_SCRIPT_TYPE_NSL)
                {
                     denginescene_ecs_do_script_entity(child, &scripts[i], callfunc, child);
                }
            }
        }
        _denginescene_do_check_script(child, scene, callfunc);
    }
}

void denginescene_ecs_do_camera_scene(Entity* camera, Scene* scene)
{
    DENGINE_DEBUG_ENTER;

    Camera* cam = camera->camera_component->camera;
    //apply position
    //TODO : this is local position, use world pos
    memcpy(cam->position,
           camera->transform.position,
           sizeof(camera->camera_component->camera->position));

    if(camera->camera_component->last_cam)
        scene->last_cam = camera->camera_component->camera;

    vec3 front;
    denginescene_ecs_get_front(camera, front);

    //we might not have entered with fb 0, save binding for later
    Framebuffer entryfb;
    dengine_entrygl_framebuffer(GL_FRAMEBUFFER, &entryfb);

    //store entry viewport
    int x, y, w, h;
    dengine_viewport_get(&x, &y, &w, &h);

    dengine_viewport_set(0, 0,
                         cam->render_width,cam->render_height);
    dengine_camera_lookat(front, cam);
    dengine_camera_project_perspective((float)cam->render_width / (float)cam->render_height,
                                       camera->camera_component->camera);
    dengine_camera_use(cam);

    // TODO : strange case of camera with mesh comp??
//    if(camera->mesh_component)
//    {
//        _denginescene_ecs_do_camera_draw_mesh(camera, camera);
//    }

    //render scene recursive
    EntityChild* ec = scene->entities.data;
    for (size_t i = 0; i < scene->entities.count; i++)
    {
        _denginescene_ecs_do_camera_draw(camera, ec[i].child);
    }

    // draw sky
    denginescene_ecs_do_skybox_scene(scene, camera->camera_component->camera);

    //now bind what we entered with
    dengine_framebuffer_bind(GL_FRAMEBUFFER, &entryfb);

    dengine_viewport_set(x, y, w, h);
}

void _denginescene_ecs_do_light_draw_shadow_mesh(Entity* light, Entity* mesh)
{
    Shader* shader = &mesh->mesh_component->material->shader_shadow;
    if(light->light_component->type != DENGINE_LIGHT_DIR)
        shader = &mesh->mesh_component->material->shader_shadow3d;

    if(!shader->program_id)
        return;

    if(light->light_component->type == DENGINE_LIGHT_DIR)
    {
        DirLight* dl =  light->light_component->light;
        dengine_material_set_texture(&dl->shadow.shadow_map.depth, "dLightShadow", mesh->mesh_component->material);
    }else if(light->light_component->type == DENGINE_LIGHT_POINT)
    {
        PointLight* pl = light->light_component->light;
        dengine_material_set_texture(&pl->shadow.shadow_map.depth, "pLightsShadow0", mesh->mesh_component->material);
    }else if(light->light_component->type == DENGINE_LIGHT_SPOT)
    {
        SpotLight* sl = light->light_component->light;
        dengine_material_set_texture(&sl->pointLight.shadow.shadow_map.depth, "sLightsShadow0", mesh->mesh_component->material);
    }

    /* TODO: Cull front faces for shadows. Make this optional */
    int entry_cullface;
    glGetIntegerv(GL_CULL_FACE_MODE, &entry_cullface);
    DENGINE_CHECKGL;
    glCullFace(GL_FRONT);
    DENGINE_CHECKGL;

    dengine_lighting_light_shadow_draw(light->light_component->type,
                                       light->light_component->light,
                                       shader,
                                       mesh->mesh_component->mesh,
                                       mesh->transform.world_model[0]
                                       );

    glCullFace(entry_cullface);
    DENGINE_CHECKGL;
}

void _denginescene_ecs_do_light_apply(Entity* light, Entity* mesh)
{
    if(mesh->mesh_component->material)
    {
        dengine_lighting_light_apply(light->light_component->type,
                                     light->light_component->light,
                                     &mesh->mesh_component->material->shader_color);
    }
}

void _denginescene_ecs_rec_light_shadow(Entity* light, Entity* root)
{
    //draw root
    if(!light->parent && root->mesh_component )
    {
        _denginescene_ecs_do_light_draw_shadow_mesh(light, root);
    }

    size_t children_count = root->children.count;
    EntityChild* ec = root->children.data;
    for (size_t i = 0; i < children_count; i++) {
        Entity* child = ec[i].child;
        if(child->mesh_component)
        {
            _denginescene_ecs_do_light_draw_shadow_mesh(light, child);
        }
        _denginescene_ecs_rec_light_shadow(light, child);
    }
}

void _denginescene_ecs_rec_light_apply(Entity* light, Entity* root)
{
    //draw root
    if(!light->parent && root->mesh_component )
    {
        _denginescene_ecs_do_light_apply(light, root);
    }

    EntityChild* ec = root->children.data;
    size_t children_count = root->children.count;
    for (size_t i = 0; i < children_count; i++) {
        Entity* child = ec[i].child;
        if(child->mesh_component)
        {
            _denginescene_ecs_do_light_apply(light, child);
        }
        _denginescene_ecs_rec_light_apply(light, child);
    }
}

void denginescene_ecs_do_light_scene(Entity* light, Scene* scene)
{
    DENGINE_DEBUG_ENTER;

    vec4 t;
    vec3 s;
    mat4 r;
    glm_decompose(light->transform.world_model, t, r, s);

    // resolve positions from ecs
    if(light->light_component->type == DENGINE_LIGHT_DIR)
    {
        DirLight* dl = light->light_component->light;
        memcpy(dl->position, t, sizeof(vec3));
        dengine_lighting_shadowop_clear(&dl->shadow);
    }else if(light->light_component->type == DENGINE_LIGHT_POINT)
    {
        PointLight* pl = light->light_component->light;
        memcpy(pl->position, t, sizeof(vec3));
        dengine_lighting_shadowop_clear(&pl->shadow);
    }else if(light->light_component->type == DENGINE_LIGHT_SPOT)
    {
        SpotLight* sl = light->light_component->light;
        memcpy(sl->pointLight.position, t, sizeof(vec3));
        dengine_lighting_shadowop_clear(&sl->pointLight.shadow);
    }

    EntityChild* ec = scene->entities.data;
    size_t children_count = scene->entities.count;

    for (size_t i = 0; i < children_count; i++)
    {
        _denginescene_ecs_rec_light_shadow(light, ec[i].child);
    }

    for (size_t i = 0; i < children_count; i++)
    {
        _denginescene_ecs_rec_light_apply(light, ec[i].child);
    }
}

void denginescene_ecs_do_skybox_scene(Scene* scene, Camera* camera)
{
    DENGINE_DEBUG_ENTER;

    if(!scene->skybox)
        return;

    // draw as the last entity
    int entrydfunc;
    glGetIntegerv(GL_DEPTH_FUNC, &entrydfunc);
    DENGINE_CHECKGL;

    int entrycullface;
    glGetIntegerv(GL_CULL_FACE_MODE, &entrycullface);
    DENGINE_CHECKGL;

    glCullFace(GL_FRONT);
    DENGINE_CHECKGL;
    glDepthFunc(GL_LEQUAL);
    DENGINE_CHECKGL;

    dengine_camera_apply(&scene->skybox->material->shader_color, camera);
    dengine_material_use(scene->skybox->material);

    dengine_draw_primitive(scene->skybox->cube, &scene->skybox->material->shader_color);

    glDepthFunc(entrydfunc);
    DENGINE_CHECKGL;
    glCullFace(entrycullface);
    DENGINE_CHECKGL;
}

void denginescene_ecs_do_scripts_entity(Entity* entity, ScriptFunc func, void* args)
{
    DENGINE_DEBUG_ENTER;

    Script* scripts = entity->scripts.data;
    for(size_t i = 0; i < entity->scripts.count; i++)
    {
        denginescene_ecs_do_script_entity(entity, &scripts[i], func, args);
    }
}

void denginescene_ecs_do_script_entity(Entity* entity, const Script* script, ScriptFunc func, void* args)
{
    DENGINE_DEBUG_ENTER;

    if(script->type == DENGINE_SCRIPT_TYPE_PYTHON)
    {
        #ifdef DENGINE_SCRIPTING_PYTHON
        PyObject* dummyentityobj = (PyObject*) args;
        denginescript_pymod_scene_entity_pull(dummyentityobj, entity);
        denginescript_python_call(script, func, dummyentityobj);
        denginescript_pymod_scene_entity_push(dummyentityobj, entity);
        #endif
    }else if(script->type == DENGINE_SCRIPT_TYPE_NSL)
    {
        denginescript_call(script, func, args);
    }
}

void denginescene_ecs_do_script_scene(Scene* scene, ScriptFunc func)
{
    DENGINE_DEBUG_ENTER;

    EntityChild* ec = scene->entities.data;
    for (uint32_t i = 0; i < scene->entities.count; i++) {
        Entity* root = ec[i].child;
        _denginescene_do_check_script(root, scene, func);
    }
}
