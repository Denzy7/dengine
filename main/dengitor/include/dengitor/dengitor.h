#ifndef DENGITOR_H
#define DENGITOR_H

#include <gtk/gtk.h>
#include <dengine/dengine.h>
#include <cglm/cglm.h>

#include "dengitor/inspector.h"
#include "dengitor/prefs.h"

typedef enum
{
    DENGITOR_GLAREA_MODE_SCENE,
    DENGITOR_GLAREA_MODE_GAME
}GLAreaMode;

typedef struct
{
    GtkApplicationWindow* main;
    GtkAboutDialog* about;
    GtkBuilder* builder;

    GtkGLArea* glarea;
    GtkAllocation* glarea_alloc;
    GtkEventBox* glarea_evbox;
    GLAreaMode glarea_mode;
    GtkToggleButton* toggle_scene;
    GtkToggleButton* toggle_game;

    Inspector inspector;
    Prefs prefs;

    GtkDialog* viewport_opts;
    GtkAdjustment* viewport_opts_fov;
    GtkAdjustment* viewport_opts_grid_width;
    GtkAdjustment* viewport_opts_grid_scale;
    GtkColorButton* viewport_opts_grid_colour;
    GtkToggleButton* viewport_opts_grid_draw;

    Entity* scene_camera;
    Scene* scene_current;
    Entity* scene_entity_current;
    float scene_entity_current_axis_width;
    int scene_camera_last_w;
    int scene_camera_last_h;

    Primitive scene_grid;
    int scene_grid_draw;
    float scene_grid_scale;
    float scene_grid_width;
    vec3 scene_grid_color;

    Primitive scene_axis;
    float scene_axis_width;
    float scene_axis_scale;

    GtkTreeView* scene_treeview;
    GtkTreeStore* scene_treeview_store;

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

void dengitor_glarea_onrealize(GtkGLArea* area);

void dengitor_glarea_onunrealize(GtkGLArea* area);

void dengitor_glarea_onrender(GtkGLArea* area);

void dengitor_toggle_scenegame_ontoggle(GtkToggleButton* toggle_btn, gpointer flag);

void dengitor_scene_treeview_oncursorchange(GtkTreeView* tree);

void dengitor_draw_axis(Primitive* axis, Shader* shader);

void dengitor_viewport_opts_setup(GtkBuilder* builder);

void dengitor_viewport_opts_grid_draw_ontoggle(GtkToggleButton* toggle_btn);

void dengitor_viewport_opts_show(GtkDialog* viewportopts);

#endif // DENGITOR_H
