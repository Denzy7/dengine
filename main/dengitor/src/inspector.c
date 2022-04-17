#include "dengitor/inspector.h"
#include "dengitor/utils.h"

void dengitor_inspector_setup(GtkBuilder* builder, Inspector* inspector)
{
    GtkContainer* transform_widget_root,* camera_widget_root;
    inspector->inspector = GTK_CONTAINER(gtk_builder_get_object(builder, "inspector_vbox"));

    inspector->transform_widget.transform = GTK_WIDGET( gtk_builder_get_object(builder, "transform_component") );
    transform_widget_root = GTK_CONTAINER(  gtk_widget_get_parent(inspector->transform_widget.transform) );
    dengitor_utils_changecontainer(inspector->transform_widget.transform,
                                   transform_widget_root, inspector->inspector);
    g_object_unref(transform_widget_root);
    inspector->transform_widget.transform_position = GTK_CONTAINER( gtk_builder_get_object(builder, "transform_component_pos") );
    inspector->transform_widget.transform_rotation = GTK_CONTAINER( gtk_builder_get_object(builder, "transform_component_rot") );
    inspector->transform_widget.transform_scale = GTK_CONTAINER( gtk_builder_get_object(builder, "transform_component_scl") );

    inspector->camera_widget.camera = GTK_WIDGET( gtk_builder_get_object(builder, "camera_component") );
    camera_widget_root = GTK_CONTAINER(  gtk_widget_get_parent(inspector->camera_widget.camera) );
    dengitor_utils_changecontainer(inspector->camera_widget.camera,
                                   camera_widget_root, inspector->inspector);
    g_object_unref(camera_widget_root);
    inspector->camera_widget.camera_fov = GTK_ADJUSTMENT( gtk_builder_get_object(builder, "camera_component_scale_adjustment") );
    inspector->camera_widget.camera_near = GTK_ENTRY( gtk_builder_get_object(builder, "camera_component_near") );
    inspector->camera_widget.camera_far = GTK_ENTRY( gtk_builder_get_object(builder, "camera_component_far") );
    inspector->camera_widget.camera_clearcolour = GTK_COLOR_BUTTON( gtk_builder_get_object(builder, "camera_component_clearcol") );
}
