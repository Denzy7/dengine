#ifndef DENGITOR_INSPECTOR_H
#define DENGITOR_INSPECTOR_H

#include <gtk/gtk.h>
#include <dengine/dengine.h>

typedef struct
{
    GtkWidget* transform;

    GtkContainer* transform_position;
    gulong sigids_transform_position[3];
    GtkContainer* transform_rotation;
    GtkContainer* transform_scale;
}TransformWidget;

typedef struct
{
    GtkWidget* camera;

    GtkAdjustment* camera_fov;
    GtkEntry* camera_near;
    GtkEntry* camera_far;
    GtkColorButton* camera_clearcolour;
}CameraWidget;

typedef struct
{
    GtkContainer* inspector;

    TransformWidget transform_widget;
    CameraWidget camera_widget;
}Inspector;

void dengitor_inspector_setup(GtkBuilder* builder, Inspector* inspector);

void dengitor_inspector_do_entity(Entity* entity, Inspector* inspector);

#endif // DENGITOR_INSPECTOR_H
