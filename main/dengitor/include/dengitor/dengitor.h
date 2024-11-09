#ifndef DENGITOR_H
#define DENGITOR_H

#include <gtk/gtk.h>
#include <dengine/dengine.h>
#include <cglm/cglm.h>

#include "dengitor/inspector.h"
#include "dengitor/prefs.h"
#include "dengitor/viewportopts.h"
#include "dengitor/scenetree.h"
#include "dengitor/w2v.h"
#include "dengitor/utils.h"
#include "dengitor/viewportopts.h"
#include "dengitor/glarea.h"
#include "dengitor/log.h"

typedef void(*Dengitor_Callback_OnEvent)();

typedef struct
{
    GtkApplicationWindow* main;
    GtkBuilder* builder;
    int activated;

    GdkCursor* cursor_blank;
    GdkCursor* cursor_arrow;

    GtkToggleButton* toggle_scene;
    GtkToggleButton* toggle_game;

    DengitorLog log;
    DengitorInspector inspector;
    DengitorPrefs prefs;
    DengitorViewportOpts viewportops;
    DengitorGLArea glarea;
    DengitorSceneTree tree;

    Entity* scene_camera;
    Scene* scene_current;
    float scene_entity_current_axis_width;
    int scene_camera_last_w;
    int scene_camera_last_h;

    Primitive scene_grid;
    int scene_grid_draw;
    float scene_grid_scale;
    float scene_grid_width;
    vec4 scene_grid_color;

    Primitive scene_axis;
    float scene_axis_width;
    float scene_axis_scale;

    StandardInput stdinput;
    NSL nsl_dengitor;

    vtor oneventcbs;

    Shader shader_default;
    Shader shader_standard;
    Shader shader_shadow2d;
    Shader shader_shadow3d;
    Shader shader_debug_normals;
    Shader shader_skybox_cube;
    Shader shader_skybox_2d;
}Dengitor;

Dengitor* dengitor_get();

/* essentially when editor runs an update step */
void dengitor_onevent_addcallback(Dengitor_Callback_OnEvent callback);

void dengitor_redraw();

#endif // DENGITOR_H
