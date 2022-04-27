#ifndef DENGITOR_INSPECTOR_H
#define DENGITOR_INSPECTOR_H

#include <gtk/gtk.h>
#include <dengine/dengine.h>

typedef struct
{
    GtkWidget* transform;

    GtkContainer* transform_position;
    GtkContainer* transform_rotation;
    GtkContainer* transform_scale;
}TransformWidget;

typedef struct
{
    GtkWidget* camera;

    GtkAdjustment* camera_fov;

    GtkEntry* camera_near;
    GtkEntry* camera_far;

    GtkEntry* camera_width;
    GtkEntry* camera_height;
    GtkButton* camera_resize;

    GtkColorButton* camera_clearcolour;
}CameraWidget;

typedef struct
{
    GtkWidget* light;

    GtkComboBox* light_type;

    GtkColorButton* light_ambient;
    GtkColorButton* light_diffuse;
    GtkColorButton* light_specular;
    GtkAdjustment* light_strength;

    GtkComboBox* light_shadow_mode;
    GtkEntry* light_shadow_size;
    GtkButton* light_shadow_resize;

    GtkToggleButton* light_shadow_pcf;
    GtkAdjustment* light_shadow_pcf_samples;

}LightWidget;

typedef struct
{
    GtkContainer* inspector;

    TransformWidget transform_widget;
    CameraWidget camera_widget;
    LightWidget light_widget;
}Inspector;

void dengitor_inspector_setup(GtkBuilder* builder, Inspector* inspector);

void dengitor_inspector_do_entity(Entity* entity, Inspector* inspector);

#endif // DENGITOR_INSPECTOR_H
