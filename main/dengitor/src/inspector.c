#include "dengitor/inspector.h"
#include "dengitor/utils.h"

GtkGLArea* glarea = NULL;

void _dengine_inspector_entry_float(GtkEntry* entry, float* val)
{
    const char* text;
    GtkEntryBuffer* buffer;
    buffer = gtk_entry_get_buffer(entry);
    text = gtk_entry_buffer_get_text(buffer);
    *val = strtof(text, NULL);

    if(glarea)
        gtk_widget_queue_draw( GTK_WIDGET(glarea) );
}

void _dengine_inspector_adjustment_float(GtkAdjustment* adjustment, float* val)
{
    *val = gtk_adjustment_get_value(adjustment);

    if(glarea)
        gtk_widget_queue_draw( GTK_WIDGET(glarea) );
}

void dengitor_inspector_setup(GtkBuilder* builder, Inspector* inspector, GtkGLArea* glarea_to_redraw_on_change)
{
    glarea = glarea_to_redraw_on_change;
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

    // we outta here...
    if(!entity)
        return;

    gtk_widget_show_all(inspector->transform_widget.transform);

    //pull transform data
    GList* pos_list = gtk_container_get_children(inspector->transform_widget.transform_position);
    GList* rot_list = gtk_container_get_children(inspector->transform_widget.transform_rotation);
    GList* scl_list = gtk_container_get_children(inspector->transform_widget.transform_scale);

    GtkEntry* entry;
    //const char* text;
    GtkEntryBuffer* buffer;
    char prtbf[1024];
    gulong* sigid;

    for(guint i = 0; i < 3; i++)
    {
        GList* pos_nth = g_list_nth(pos_list, i);
        GList* rot_nth = g_list_nth(rot_list, i);
        GList* scl_nth = g_list_nth(scl_list, i);

        // pos
        entry = GTK_ENTRY(pos_nth->data);
        buffer = gtk_entry_get_buffer(entry);

        // sigchange
        sigid = &inspector->transform_widget.sigids_transform_position[i];
        if(*sigid)
        {
            g_signal_handler_disconnect(entry, *sigid);
        }
        *sigid = g_signal_connect(entry, "changed",
                         G_CALLBACK(_dengine_inspector_entry_float), &entity->transform.position[i]);

        g_snprintf(prtbf, sizeof(prtbf), "%.1f", entity->transform.position[i]);
        gtk_entry_buffer_set_text(buffer, prtbf, strlen(prtbf));

        //rot
        entry = GTK_ENTRY(rot_nth->data);
        buffer = gtk_entry_get_buffer(entry);

        //sigchanged
        sigid = &inspector->transform_widget.sigids_transform_rotation[i];
        if(*sigid)
        {
            g_signal_handler_disconnect(entry, *sigid);
        }
        *sigid = g_signal_connect(entry, "changed",
                         G_CALLBACK(_dengine_inspector_entry_float), &entity->transform.rotation[i]);

        g_snprintf(prtbf, sizeof(prtbf), "%.1f", entity->transform.rotation[i]);
        gtk_entry_buffer_set_text(buffer, prtbf, strlen(prtbf));

        //scl
        entry = GTK_ENTRY(scl_nth->data);
        buffer = gtk_entry_get_buffer(entry);

        sigid = &inspector->transform_widget.sigids_transform_scale[i];
        if(*sigid)
        {
            g_signal_handler_disconnect(entry, *sigid);
        }
        *sigid = g_signal_connect(entry, "changed",
                         G_CALLBACK(_dengine_inspector_entry_float), &entity->transform.scale[i]);

        g_snprintf(prtbf, sizeof(prtbf), "%.1f", entity->transform.scale[i]);
        gtk_entry_buffer_set_text(buffer, prtbf, strlen(prtbf));
    }

    if(entity->camera_component)
    {
        Camera* camera = entity->camera_component->camera;
        gtk_widget_show_all(inspector->camera_widget.camera);

        sigid = &inspector->camera_widget.sigid_camera_fov;
        if(*sigid)
        {
            g_signal_handler_disconnect(
                        inspector->camera_widget.camera_fov, *sigid);
        }
        *sigid = g_signal_connect(inspector->camera_widget.camera_fov, "value-changed",
                         G_CALLBACK(_dengine_inspector_adjustment_float), &camera->fov);
        // fov
        gtk_adjustment_set_value(
                    inspector->camera_widget.camera_fov,
                    camera->fov);

        //near
        entry = inspector->camera_widget.camera_near;
        buffer = gtk_entry_get_buffer(entry);

        sigid = &inspector->camera_widget.sigid_camera_near;
        if(*sigid)
        {
            g_signal_handler_disconnect(entry, *sigid);
        }
        *sigid = g_signal_connect(entry, "changed",
                         G_CALLBACK(_dengine_inspector_entry_float), &camera->near);

        g_snprintf(prtbf, sizeof(prtbf), "%.2f", camera->near);
        gtk_entry_buffer_set_text(buffer, prtbf, strlen(prtbf));

        //far
        entry = inspector->camera_widget.camera_far;
        buffer = gtk_entry_get_buffer(entry);

        sigid = &inspector->camera_widget.sigid_camera_far;
        if(*sigid)
        {
            g_signal_handler_disconnect(entry, *sigid);
        }
        *sigid = g_signal_connect(entry, "changed",
                         G_CALLBACK(_dengine_inspector_entry_float), &camera->far);

        g_snprintf(prtbf, sizeof(prtbf), "%.2f", camera->far);
        gtk_entry_buffer_set_text(buffer, prtbf, strlen(prtbf));

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
