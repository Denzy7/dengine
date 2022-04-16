#ifndef DENGITOR_H
#define DENGITOR_H

#include <gtk/gtk.h>
#include <dengine/dengine.h>

typedef struct
{
    GtkApplicationWindow* main;
    GtkAboutDialog* about;
    GtkBuilder* builder;

    GtkGLArea* scene_glarea;

    Entity* scene_camera;
    Scene* scene_current;
}Dengitor;

void dengitor_onactivate(GtkApplication* app);

void dengitor_aboutdialog_show();

void dengitor_aboutdialog_hide();

void dengitor_scene_glarea_onrealize(GtkGLArea* area);

void dengitor_scene_glarea_onunrealize(GtkGLArea* area);

void dengitor_scene_glarea_onrender(GtkGLArea* area);
#endif // DENGITOR_H
