#ifndef DENGITOR_H
#define DENGITOR_H

#include <gtk/gtk.h>
#include <dengine/dengine.h>
#include <cglm/cglm.h>

typedef struct
{
    GtkApplicationWindow* main;
    GtkAboutDialog* about;
    GtkBuilder* builder;

    GtkGLArea* scene_glarea;

    Entity* scene_camera;
    Scene* scene_current;

    Primitive scene_grid;
    float scene_grid_scale;
    float scene_grid_width;
    vec3 scene_grid_color;

    Primitive scene_axis;
    float scene_axis_width;
    float scene_axis_scale;

    Shader* shader_default;
    Shader* shader_standard;
    Shader* shader_shadow2d;
    Shader* shader_shadow3d;
    Shader* shader_debug_normals;
    Shader* shader_skybox_cube;
    Shader* shader_skybox_2d;
}Dengitor;

void dengitor_onactivate(GtkApplication* app);

void dengitor_aboutdialog_show();

void dengitor_aboutdialog_hide();

void dengitor_scene_glarea_onrealize(GtkGLArea* area);

void dengitor_scene_glarea_onunrealize(GtkGLArea* area);

void dengitor_scene_glarea_onrender(GtkGLArea* area);

void dengitor_draw_axis(Primitive* axis, Shader* shader);
#endif // DENGITOR_H
