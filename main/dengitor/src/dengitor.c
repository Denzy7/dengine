#include "dengitor/dengitor.h"


extern unsigned char breeze_zip[];
extern unsigned int breeze_zip_ln;

static Dengitor* dengitor = NULL;

Dengitor* dengitor_get()
{
    return dengitor;
}

DENGINE_EXPORT int dengitor_onevent()
{
    Dengitor_Callback_OnEvent* cbs = dengitor->oneventcbs.data;
    for(size_t i = 0; i < dengitor->oneventcbs.count; i++)
    {
        cbs[i]();
    }

    dengineutils_timer_update();

    dengitor_redraw();
    return FALSE;
}

DENGINE_EXPORT gboolean dengitor_widget_hide_ondelete(GtkWidget* widget)
{
    gtk_widget_hide_on_delete(GTK_WIDGET(widget));
    return TRUE;
}

void dengitor_onactivate(GtkApplication* app)
{
    // MAIN UI
    dengitor->builder = gtk_builder_new_from_resource("/com/denzygames/Dengitor/dengine-editor-ui.glade");
    gtk_builder_connect_signals(dengitor->builder, NULL);

    dengitor->main = GTK_APPLICATION_WINDOW(gtk_builder_get_object(dengitor->builder, "main"));
    /*gtk_widget_add_tick_callback( GTK_WIDGET(dengitor->main), dengitor_main_ontick, NULL, NULL);*/

    // a blank cursor for hiding when dragging glarea
    // TODO : will this cause issues with multi-displays?
    dengitor->cursor_blank = gdk_cursor_new_for_display( gdk_display_get_default(), GDK_BLANK_CURSOR);
    dengitor->cursor_arrow = gdk_cursor_new_for_display( gdk_display_get_default(), GDK_ARROW);

    //  ABOUT WINDOW
    gtk_about_dialog_set_version(
            GTK_ABOUT_DIALOG(gtk_builder_get_object(dengitor->builder, "about")), DENGINE_VERSION);
    gtk_about_dialog_set_license(
            GTK_ABOUT_DIALOG(gtk_builder_get_object(dengitor->builder, "about")), dengine_get_license());

    
    //  PREFS WINDOW
    Stream breeze_stream;
    ZipRead breeze_zipread;
    CDFHR* cdfhr;
    
    dengineutils_stream_new_mem(breeze_zip, breeze_zip_ln, &breeze_stream);
    dengineutils_zipread_load(&breeze_stream, &breeze_zipread);
    dengineutils_zipread_find_cdfhr("dark.css", &cdfhr, &breeze_zipread);
    dengineutils_zipread_decompress_cdfhr_mem(&breeze_stream, cdfhr, (void**)&dengitor->prefs.breeze_dark, &dengitor->prefs.breeze_dark_sz);
    dengineutils_zipread_find_cdfhr("light.css", &cdfhr, &breeze_zipread);
    dengineutils_zipread_decompress_cdfhr_mem(&breeze_stream, cdfhr, (void**)&dengitor->prefs.breeze_light, &dengitor->prefs.breeze_light_sz);
    dengineutils_zipread_free(&breeze_zipread);

    
    GtkSettings* settings = gtk_settings_get_default();
    g_object_set(settings, "gtk-xft-antialias", 1, NULL);
    g_object_set(settings, "gtk-xft-hinting", 1, NULL);
    g_object_set(settings, "gtk-xft-hintstyle", "hintslight", NULL);

    dengitor->activated = 1;

    dengitor_scenetree_setup(dengitor->builder, &dengitor->tree);
    dengitor_prefs_setup(dengitor->builder, &dengitor->prefs);
    dengitor_inspector_setup(dengitor->builder, &dengitor->inspector);
    dengitor_viewportopts_setup(dengitor->builder, &dengitor->viewportops);
    dengitor_glarea_setup(dengitor->builder, &dengitor->glarea);

    /* gtk connect's signals on show? we need to show window last */
    gtk_application_add_window(app, GTK_WINDOW(dengitor->main));
    gtk_widget_show_all(GTK_WIDGET(dengitor->main));

    gtk_window_maximize( GTK_WINDOW(dengitor->main) ) ;
    char prtbf[1024];
    snprintf(prtbf, sizeof(prtbf), "Dengitor - v%s --  GL : %s, RENDERER : %s",
             DENGINE_VERSION,
             (char*)glGetString(GL_VERSION),
             (char*)glGetString(GL_RENDERER));;
    gtk_window_set_title( GTK_WINDOW(dengitor->main), prtbf);

    // hide inspector
    dengitor_inspector_do_entity(NULL, &dengitor->inspector);
}

void dengitor_redraw()
{
    gtk_widget_queue_draw(GTK_WIDGET(dengitor->main));
}
int main(int argc, char *argv[])
{
    dengitor = calloc(1, sizeof(Dengitor));

    dengitor->scene_grid_scale = 10.0f;
    dengitor->scene_grid_width = 0.5;
    dengitor->scene_grid_color[0] = 1.0f;
    dengitor->scene_grid_color[1] = 1.0f;
    dengitor->scene_grid_color[2] = 0.0f;
    dengitor->scene_grid_color[3] = 1.0f;
    dengitor->scene_grid_draw = 1;

    dengitor->scene_axis_scale = 2.0f;
    dengitor->scene_axis_width = 3.5f;
    dengitor->scene_entity_current_axis_width = 1.5;
    dengitor->viewportops.viewport_padding = 5.0f;

    vtor_create_ptrs(&dengitor->oneventcbs);
    if((dengitor->nsl_dengitor = denginescript_nsl_load("nsl-dengitor")) == NULL)
    {
        dengineutils_logging_log("ERROR::COULD NOT LOAD nsl-dengitor needed for vital editor scripts! You may need to recompile dengitor");
        return 1;
    }

    /* default flags keep changing. but 0 is default*/
    GtkApplication* app = gtk_application_new(
                "com.denzygames.Dengitor", 0);
    g_signal_connect(app, "activate", G_CALLBACK(dengitor_onactivate), NULL);
    int run = g_application_run( G_APPLICATION(app), argc, argv);

    denginescript_nsl_free(dengitor->nsl_dengitor);
    vtor_free(&dengitor->oneventcbs);

    //app exiting
    if(dengitor)
        free(dengitor);

    return run;
}

void dengitor_onevent_addcallback(Dengitor_Callback_OnEvent callback)
{
    vtor_pushback(&dengitor->oneventcbs, callback);
}
