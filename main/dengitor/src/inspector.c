#include "dengitor/inspector.h"
#include "dengitor/utils.h"
#include "dengitor/w2v.h"

Entity* current_entity = NULL;
ShadowOp* current_shadowop = NULL;
uint32_t current_shadowop_tgt = 0;

void _dengine_inspector_camera_resize(GtkButton* button, Inspector* inspector)
{
    int w, h;
    GtkEntryBuffer* buffer = gtk_entry_get_buffer(inspector->camera_widget.camera_width);
    sscanf(gtk_entry_buffer_get_text( buffer ), "%d", &w);

    buffer = gtk_entry_get_buffer(inspector->camera_widget.camera_height);
    sscanf(gtk_entry_buffer_get_text( buffer ), "%d", &h);

    if(current_entity && current_entity->camera_component)
    {
        dengine_camera_resize(current_entity->camera_component->camera, w, h);
        dengineutils_logging_log("INFO::resize camera to %dx%d", w, h);
    }
}

void _dengine_inspector_shadowop_resize(GtkButton* button, Inspector* inspector)
{
    int sz;
    GtkEntryBuffer* buffer = gtk_entry_get_buffer(inspector->light_widget.light_shadow_size);
    sscanf(gtk_entry_buffer_get_text( buffer ), "%d", &sz);

    if(current_shadowop)
    {
        dengine_lighting_shadowop_resize(current_shadowop_tgt, current_shadowop, sz);
        dengineutils_logging_log("INFO::resize shadowop to %dx%d", sz, sz);
    }
}

void dengitor_inspector_setup(GtkBuilder* builder, Inspector* inspector)
{
    GtkContainer* transform_widget_root,* camera_widget_root,* light_widget_root;
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

    inspector->light_widget.light = GTK_WIDGET( gtk_builder_get_object(builder, "light_component") );
    inspector->light_widget.light_ambient = GTK_COLOR_BUTTON( gtk_builder_get_object(builder, "light_component_ambient") );
    inspector->light_widget.light_diffuse = GTK_COLOR_BUTTON( gtk_builder_get_object(builder, "light_component_diffuse") );
    inspector->light_widget.light_specular = GTK_COLOR_BUTTON( gtk_builder_get_object(builder, "light_component_specular") );
    inspector->light_widget.light_strength = GTK_ADJUSTMENT( gtk_builder_get_object(builder, "light_component_strength_adjustment") );
    inspector->light_widget.light_shadow_size = GTK_ENTRY( gtk_builder_get_object(builder, "light_component_shadow_size") );
    inspector->light_widget.light_shadow_resize = GTK_BUTTON ( gtk_builder_get_object(builder, "light_component_shadow_resize") );
    inspector->light_widget.light_shadow_pcf = GTK_TOGGLE_BUTTON( gtk_builder_get_object(builder, "light_component_shadow_pcf") );
    inspector->light_widget.light_shadow_pcf_samples = GTK_ADJUSTMENT( gtk_builder_get_object(builder, "light_component_shadow_pcf_samples") );

    light_widget_root = GTK_CONTAINER( gtk_widget_get_parent(inspector->light_widget.light) );
    dengitor_utils_changecontainer(inspector->light_widget.light,
                                   light_widget_root, inspector->inspector);
    g_object_unref(light_widget_root);

}

void dengitor_inspector_do_entity(Entity* entity, Inspector* inspector)
{
    // hide all. only show if we have the component
    gtk_widget_hide(inspector->camera_widget.camera);
    gtk_widget_hide(inspector->transform_widget.transform);
    gtk_widget_hide(inspector->light_widget.light);

    current_entity = entity;

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
    GtkColorButton* colorbtn;
    GtkEntryBuffer* buffer;
    GtkToggleButton* toggle;
    GdkRGBA rgba;
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
        dengitor_utils_float4_to_rgba(camera->clearcolor, &rgba);
        gtk_color_chooser_set_rgba(
                    GTK_COLOR_CHOOSER(inspector->camera_widget.camera_clearcolour),
                    &rgba);
    }

    if(entity->light_component)
    {
        LightOp* light_op = NULL;
        ShadowOp* shadow_op = NULL;

        if(entity->light_component->type == DENGINE_LIGHT_DIR)
        {
            light_op = &((DirLight*)entity->light_component->light)->light;
            shadow_op = &((DirLight*)entity->light_component->light)->shadow;
            current_shadowop_tgt = GL_TEXTURE_2D;
        }else if(entity->light_component->type == DENGINE_LIGHT_POINT)
        {
            light_op = &((PointLight*)entity->light_component->light)->light;
            shadow_op = &((PointLight*)entity->light_component->light)->shadow;
            current_shadowop_tgt = GL_TEXTURE_CUBE_MAP;
        }else if(entity->light_component->type == DENGINE_LIGHT_SPOT)
        {
            light_op = &((SpotLight*)entity->light_component->light)->pointLight.light;
            shadow_op = &((SpotLight*)entity->light_component->light)->pointLight.shadow;
            current_shadowop_tgt = GL_TEXTURE_CUBE_MAP;
        }

        gtk_widget_show_all(inspector->light_widget.light);

        if(light_op)
        {
            //ambient
            colorbtn = inspector->light_widget.light_ambient;
            dengitor_utils_disconnect(gtk_color_button_get_type(), colorbtn, "color-set");
            g_signal_connect(colorbtn, "color-set",
                              G_CALLBACK(dengitor_w2v_colorbtn2float4), light_op->ambient);
            dengitor_utils_float4_to_rgba(light_op->ambient, &rgba);
            gtk_color_chooser_set_rgba( GTK_COLOR_CHOOSER(colorbtn), &rgba );

            //diffuse
            colorbtn = inspector->light_widget.light_diffuse;
            dengitor_utils_disconnect(gtk_color_button_get_type(), colorbtn, "color-set");
            g_signal_connect(colorbtn, "color-set",
                              G_CALLBACK(dengitor_w2v_colorbtn2float4), light_op->diffuse);
            dengitor_utils_float4_to_rgba(light_op->diffuse, &rgba);
            gtk_color_chooser_set_rgba( GTK_COLOR_CHOOSER(colorbtn), &rgba );

            //specular
            colorbtn = inspector->light_widget.light_specular;
            dengitor_utils_disconnect(gtk_color_button_get_type(), colorbtn, "color-set");
            g_signal_connect(colorbtn, "color-set",
                              G_CALLBACK(dengitor_w2v_colorbtn2float4), light_op->specular);
            dengitor_utils_float4_to_rgba(light_op->specular, &rgba);
            gtk_color_chooser_set_rgba( GTK_COLOR_CHOOSER(colorbtn), &rgba );

            //strength
            adjustment = inspector->light_widget.light_strength;
            dengitor_utils_disconnect(gtk_adjustment_get_type(), adjustment, "value-changed");
            g_signal_connect(adjustment, "value-changed",
                             G_CALLBACK(dengitor_w2v_adjustment2float), &light_op->strength);
            gtk_adjustment_set_value(adjustment, light_op->strength);


        }

        if(shadow_op)
        {
            current_shadowop = shadow_op;

            //shadow sz
            entry = inspector->light_widget.light_shadow_size;
            buffer = gtk_entry_get_buffer(entry);
            g_snprintf(prtbf, sizeof(prtbf), "%d", shadow_op->shadow_map_size);
            gtk_entry_buffer_set_text(buffer, prtbf, strlen(prtbf));

            //shadow res
            button = inspector->light_widget.light_shadow_resize;
            dengitor_utils_disconnect(gtk_button_get_type(), button, "clicked");
            g_signal_connect(button, "clicked",
                             G_CALLBACK(_dengine_inspector_shadowop_resize), inspector);

            //shadow pcf
            toggle = inspector->light_widget.light_shadow_pcf;
            dengitor_utils_disconnect(gtk_toggle_button_get_type(), toggle, "toggled");
            g_signal_connect(toggle, "toggled",
                             G_CALLBACK(dengitor_w2v_toggle2int), &shadow_op->pcf);

            //shadow pcf sampling
            adjustment = inspector->light_widget.light_shadow_pcf_samples;
            dengitor_utils_disconnect(gtk_adjustment_get_type(), adjustment, "value-changed");
            g_signal_connect(adjustment, "value-changed",
                             G_CALLBACK(dengitor_w2v_adjustment2int), &shadow_op->pcf_samples);
            gtk_adjustment_set_value(adjustment, (gdouble) shadow_op->pcf_samples);

        }
    }
}
