#include "dengitor/inspector.h"
#include "dengitor/utils.h"
#include "dengitor/w2v.h"
Entity* current = NULL;

void _dengine_inspector_camera_resize(GtkButton* button, Inspector* inspector)
{
    int w, h;
    GtkEntryBuffer* buffer = gtk_entry_get_buffer(inspector->camera_widget.camera_width);
    sscanf(gtk_entry_buffer_get_text( buffer ), "%d", &w);

    buffer = gtk_entry_get_buffer(inspector->camera_widget.camera_height);
    sscanf(gtk_entry_buffer_get_text( buffer ), "%d", &h);

    if(current && current->camera_component)
    {
        dengine_camera_resize(current->camera_component->camera, w, h);
        dengineutils_logging_log("INFO::resize camera to %dx%d", w, h);
    }
}

void dengitor_inspector_setup(GtkBuilder* builder, Inspector* inspector)
{
    GtkContainer* transform_widget_root,* camera_widget_root;
    inspector->inspector = GTK_CONTAINER(gtk_builder_get_object(builder, "inspector_vbox"));

    inspector->transform_widget.transform = GTK_WIDGET( gtk_builder_get_object(builder, "transform_component") );
    inspector->transform_widget.transform_position = GTK_CONTAINER( gtk_builder_get_object(builder, "transform_component_pos") );
    inspector->transform_widget.transform_rotation = GTK_CONTAINER( gtk_builder_get_object(builder, "transform_component_rot") );
    inspector->transform_widget.transform_scale = GTK_CONTAINER( gtk_builder_get_object(builder, "transform_component_scl") );

    transform_widget_root = GTK_CONTAINER(  gtk_widget_get_parent(inspector->transform_widget.transform) );
    dengitor_utils_changecontainer(inspector->transform_widget.transform,
                                   transform_widget_root, inspector->inspector);
    g_object_unref(transform_widget_root);

    inspector->camera_widget.camera = GTK_WIDGET( gtk_builder_get_object(builder, "camera_component") );
    inspector->camera_widget.camera_fov = GTK_ADJUSTMENT( gtk_builder_get_object(builder, "camera_component_scale_adjustment") );
    inspector->camera_widget.camera_near = GTK_ENTRY( gtk_builder_get_object(builder, "camera_component_near") );
    inspector->camera_widget.camera_far = GTK_ENTRY( gtk_builder_get_object(builder, "camera_component_far") );
    inspector->camera_widget.camera_width = GTK_ENTRY( gtk_builder_get_object(builder, "camera_component_width") );
    inspector->camera_widget.camera_height = GTK_ENTRY( gtk_builder_get_object(builder, "camera_component_height") );
    inspector->camera_widget.camera_resize = GTK_BUTTON( gtk_builder_get_object(builder, "camera_component_resize") );
    inspector->camera_widget.camera_clearcolour = GTK_COLOR_BUTTON( gtk_builder_get_object(builder, "camera_component_clearcol") );

    camera_widget_root = GTK_CONTAINER(  gtk_widget_get_parent(inspector->camera_widget.camera) );
    dengitor_utils_changecontainer(inspector->camera_widget.camera,
                                   camera_widget_root, inspector->inspector);
    g_object_unref(camera_widget_root);
}

void dengitor_inspector_do_entity(Entity* entity, Inspector* inspector)
{
    // hide all. only show if we have the component
    gtk_widget_hide(inspector->camera_widget.camera);
    gtk_widget_hide(inspector->transform_widget.transform);

    current = entity;

    // we outta here...
    if(!entity)
        return;

    gtk_widget_show_all(inspector->transform_widget.transform);

    //pull transform data
    GList* pos_list = gtk_container_get_children(inspector->transform_widget.transform_position);
    GList* rot_list = gtk_container_get_children(inspector->transform_widget.transform_rotation);
    GList* scl_list = gtk_container_get_children(inspector->transform_widget.transform_scale);

    GtkEntry* entry;
    GtkAdjustment* adjustment;
    GtkButton* button;
    //const char* text;
    GtkEntryBuffer* buffer;
    char prtbf[1024];

    /*
     * NOTE:
     *
     * Inspector uses a combination of diconnecting
     * and connecting signals to work
     *
     * This approach allows more flexibility with w2v
     * so that values can be instantly mapped to a widget
     *
     * This does come at the cost of disconnecting previous
     * signal and connecting a new one with selected entity (its
     * important it occurs in that order). Failure to will cause
     * the selected entity to pull data from previous entity
     * (WHICH YOU DONT BASICALLY WANT) or multiple callbacks!
     *
     * Then pull its data (from ecs or core structs)
     * to the widget.
     *
     * Some widgets just need to pull ecs data (like getting camera
     * render width) these do not need to connect any signals
     */

    for(guint i = 0; i < 3; i++)
    {
        GList* pos_nth = g_list_nth(pos_list, i);
        GList* rot_nth = g_list_nth(rot_list, i);
        GList* scl_nth = g_list_nth(scl_list, i);

        // pos
        entry = GTK_ENTRY(pos_nth->data);
        buffer = gtk_entry_get_buffer(entry);
        dengitor_utils_disconnect(gtk_entry_get_type(), entry, "changed");
        g_signal_connect(entry, "changed",
                         G_CALLBACK(dengitor_w2v_entry2float), &entity->transform.position[i]);
        g_snprintf(prtbf, sizeof(prtbf), "%.1f", entity->transform.position[i]);
        gtk_entry_buffer_set_text(buffer, prtbf, strlen(prtbf));

        //rot
        entry = GTK_ENTRY(rot_nth->data);
        buffer = gtk_entry_get_buffer(entry);
        dengitor_utils_disconnect(gtk_entry_get_type(), entry, "changed");
        g_signal_connect(entry, "changed",
                         G_CALLBACK(dengitor_w2v_entry2float), &entity->transform.rotation[i]);
        g_snprintf(prtbf, sizeof(prtbf), "%.1f", entity->transform.rotation[i]);
        gtk_entry_buffer_set_text(buffer, prtbf, strlen(prtbf));

        //scl
        entry = GTK_ENTRY(scl_nth->data);
        buffer = gtk_entry_get_buffer(entry);
        dengitor_utils_disconnect(gtk_entry_get_type(), entry, "changed");
        g_signal_connect(entry, "changed",
                         G_CALLBACK(dengitor_w2v_entry2float), &entity->transform.scale[i]);
        g_snprintf(prtbf, sizeof(prtbf), "%.1f", entity->transform.scale[i]);
        gtk_entry_buffer_set_text(buffer, prtbf, strlen(prtbf));
    }

    if(entity->camera_component)
    {
        Camera* camera = entity->camera_component->camera;
        gtk_widget_show_all(inspector->camera_widget.camera);

        // fov
        adjustment = inspector->camera_widget.camera_fov;
        dengitor_utils_disconnect(gtk_adjustment_get_type(), adjustment, "value-changed");
        g_signal_connect(adjustment, "value-changed",
                         G_CALLBACK(dengitor_w2v_adjustment2float),&camera->fov);
        gtk_adjustment_set_value(adjustment, camera->fov);

        //near
        entry = inspector->camera_widget.camera_near;
        buffer = gtk_entry_get_buffer(entry);
        dengitor_utils_disconnect(gtk_entry_get_type(), entry, "changed");
        g_signal_connect(entry, "changed",
                         G_CALLBACK(dengitor_w2v_entry2float), &camera->near);
        g_snprintf(prtbf, sizeof(prtbf), "%.2f", camera->near);
        gtk_entry_buffer_set_text(buffer, prtbf, strlen(prtbf));

        //far
        entry = inspector->camera_widget.camera_far;
        buffer = gtk_entry_get_buffer(entry);
        dengitor_utils_disconnect(gtk_entry_get_type(), entry, "changed");
        g_signal_connect(entry, "changed",
                         G_CALLBACK(dengitor_w2v_entry2float), &camera->far);
        g_snprintf(prtbf, sizeof(prtbf), "%.2f", camera->far);
        gtk_entry_buffer_set_text(buffer, prtbf, strlen(prtbf));

        //width
        entry = inspector->camera_widget.camera_width;
        buffer = gtk_entry_get_buffer(entry);
        g_snprintf(prtbf, sizeof(prtbf), "%d", camera->render_width);
        gtk_entry_buffer_set_text(buffer, prtbf, strlen(prtbf));

        //height
        entry = inspector->camera_widget.camera_height;
        buffer = gtk_entry_get_buffer(entry);
        g_snprintf(prtbf, sizeof(prtbf), "%d", camera->render_height);
        gtk_entry_buffer_set_text(buffer, prtbf, strlen(prtbf));

        button = inspector->camera_widget.camera_resize;
        dengitor_utils_disconnect(gtk_button_get_type(), button, "clicked");
        g_signal_connect(button, "clicked",
                         G_CALLBACK(_dengine_inspector_camera_resize), inspector);
        //clear
        GdkRGBA rgba = {camera->clearcolor[0],
                        camera->clearcolor[1],
                        camera->clearcolor[2],
                        camera->clearcolor[3],
                       };
        gtk_color_chooser_set_rgba(
                    GTK_COLOR_CHOOSER(inspector->camera_widget.camera_clearcolour),
                    &rgba);
    }
}
