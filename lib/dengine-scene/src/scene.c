#include <stdlib.h> //malloc
#include <string.h> //memset

#include "dengine-scene/scene.h"
#include "dengine-utils/logging.h"
#include "dengine/draw.h"
#include "dengine/loadgl.h" //getFBO
#include "dengine_config.h"// DENGINE_ECS_MAXCHILDREN

void _denginescene_do_check_camera(Entity* root, Scene* scene);

Scene* denginescene_new()
{
    Scene* newscn = malloc(sizeof (struct _Scene));
    memset(newscn, 0, sizeof (Scene));

    newscn->entities = malloc(DENGINE_ECS_MAXCHILDREN * sizeof (struct _Entity*));

    return newscn;
}

Skybox*
denginescene_new_skybox(const Primitive* cube, const Material* material)
{
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
    for (uint32_t i = 0; i < scene->n_entities; i++) {
        denginescene_ecs_destroy_entity(scene->entities[i]);
    }
    free(scene->entities);

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
    if (scene->n_entities >= DENGINE_ECS_MAXCHILDREN) {
        dengineutils_logging_log("WARNING::Scene limit reached! Recompile to increase limit. Free the allocated entity to avoid a memory leak!");
        return;
    }
    scene->entities[scene->n_entities] = entity;
    scene->n_entities++;
}

void denginescene_update(Scene* scene)
{
    for (uint32_t i = 0; i < scene->n_entities; i++) {
        Entity* root=scene->entities[i];
        denginescene_ecs_transform_entity(root);
        _denginescene_do_check_camera(root,scene);
    }
}

void _denginescene_ecs_do_camera_draw_mesh(Entity* camera, Entity* mesh)
{
    //dengineutils_logging_log("drw %u",mesh->entity_id);
    dengine_material_use(mesh->mesh_component->material);
    dengine_camera_lookat(NULL, camera->camera_component->camera);
    dengine_camera_apply(&mesh->mesh_component->material->shader_color,camera->camera_component->camera);

    dengine_shader_set_mat4(&mesh->mesh_component->material->shader_color,
                            "model",
                            mesh->transform.world_model[0]
                            );

    dengine_draw_primitive(mesh->mesh_component->mesh,&mesh->mesh_component->material->shader_color);
    dengine_material_use(NULL);
}

void _denginescene_ecs_do_camera_draw(Entity* camera,Entity* root)
{
    if(!root->parent && root->mesh_component)
        _denginescene_ecs_do_camera_draw_mesh(camera,root);

    size_t children_count = root->children_count;
    for (size_t i = 0; i < children_count; i++) {
        Entity* child = root->children[i];
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

    size_t children_count = root->children_count;
    for (size_t i = 0; i < children_count; i++) {
        Entity* child = root->children[i];
        if(child->camera_component)
        {
            denginescene_ecs_do_camera_scene(child,scene);
        }
        _denginescene_do_check_camera(child, scene);
    }
}

void denginescene_ecs_do_camera_scene(Entity* camera, Scene* scene)
{
    //apply position
    //TODO : this is local position, use world pos
    memcpy(camera->camera_component->camera->position,
           camera->transform.position,
           sizeof(camera->camera_component->camera->position));

    //we might not have entered with fb 0, save binding for later
    int bind;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &bind); DENGINE_CHECKGL;

    dengine_camera_use(camera->camera_component->camera);

    // TODO : strange case of camera with mesh comp??
//    if(camera->mesh_component)
//    {
//        _denginescene_ecs_do_camera_draw_mesh(camera, camera);
//    }

    //render scene recursive
    for (size_t i = 0; i < scene->n_entities; i++)
    {
        _denginescene_ecs_do_camera_draw(camera,scene->entities[i]);
    }

    // draw sky
    denginescene_ecs_do_skybox_scene(scene, camera->camera_component->camera);

    //now bind what we entered with
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, bind); DENGINE_CHECKGL;
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

    dengine_lighting_light_shadow_draw(light->light_component->type,
                                       light->light_component->light,
                                       shader,
                                       mesh->mesh_component->mesh,
                                       mesh->transform.world_model[0]
                                       );
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

    size_t children_count = root->children_count;
    for (size_t i = 0; i < children_count; i++) {
        Entity* child = root->children[i];
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

    size_t children_count = root->children_count;
    for (size_t i = 0; i < children_count; i++) {
        Entity* child = root->children[i];
        if(child->mesh_component)
        {
            _denginescene_ecs_do_light_apply(light, child);
        }
        _denginescene_ecs_rec_light_apply(light, child);
    }
}

void denginescene_ecs_do_light_scene(Entity* light, Scene* scene)
{
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

    for (size_t i = 0; i < scene->n_entities; i++)
    {
        _denginescene_ecs_rec_light_shadow(light, scene->entities[i]);
    }

    for (size_t i = 0; i < scene->n_entities; i++)
    {
        _denginescene_ecs_rec_light_apply(light, scene->entities[i]);
    }
}

void denginescene_ecs_do_skybox_scene(Scene* scene, Camera* camera)
{
    if(!scene->skybox)
        return;

    // draw as the last entity

    int entrydfunc;
    glGetIntegerv(GL_DEPTH_FUNC, &entrydfunc);

    glDepthFunc(GL_LEQUAL);

    dengine_camera_apply(&scene->skybox->material->shader_color, camera);
    dengine_material_use(scene->skybox->material);

    dengine_draw_primitive(scene->skybox->cube, &scene->skybox->material->shader_color);

    dengine_material_use(NULL);
    glDepthFunc(entrydfunc);
}
