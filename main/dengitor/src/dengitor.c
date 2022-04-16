#include <dengine/dengine.h>
#include "dengitor/dengitor.h"

static Dengitor dengitor;

int main(int argc, char *argv[])
{

    GtkApplication* app = gtk_application_new(
                "com.denzygames.Dengitor", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(dengitor_onactivate), NULL);
    int run = g_application_run( G_APPLICATION(app), argc, argv);
    return run;
}

void dengitor_onactivate(GtkApplication* app)
{
    //TODO : compile resources
    const char* ui = "/home/denzy/dengine/main/dengitor/res/default/dengine-editor-ui.glade";

    dengitor.builder = gtk_builder_new_from_file(ui);

    dengitor.main = GTK_APPLICATION_WINDOW(gtk_builder_get_object(dengitor.builder, "main"));

    dengitor.about = GTK_ABOUT_DIALOG(gtk_builder_get_object(dengitor.builder, "about"));
    g_signal_connect(dengitor.about, "response", G_CALLBACK(dengitor_aboutdialog_hide), NULL);
    // menu button
    g_signal_connect(gtk_builder_get_object(dengitor.builder, "menu_action"),
                     "activate", G_CALLBACK(dengitor_aboutdialog_show), NULL);

    dengitor.scene_glarea = GTK_GL_AREA(gtk_builder_get_object(dengitor.builder, "scene_glarea"));
    g_signal_connect(dengitor.scene_glarea,
                     "realize", G_CALLBACK(dengitor_scene_glarea_onrealize), NULL);
    g_signal_connect(dengitor.scene_glarea,
                     "unrealize", G_CALLBACK(dengitor_scene_glarea_onunrealize), NULL);
    g_signal_connect(dengitor.scene_glarea,
                     "render", G_CALLBACK(dengitor_scene_glarea_onrender), NULL);

    gtk_application_add_window(app, GTK_WINDOW(dengitor.main));
    gtk_widget_show_all(GTK_WIDGET(dengitor.main));
}

void dengitor_aboutdialog_show()
{
    gtk_about_dialog_set_version(dengitor.about, DENGINE_VERSION);
    gtk_widget_show_all(GTK_WIDGET(dengitor.about));
}

void dengitor_aboutdialog_hide()
{
    gtk_widget_hide(GTK_WIDGET(dengitor.about));
}

void dengitor_scene_glarea_onrealize(GtkGLArea* area)
{
    gtk_gl_area_make_current(area);
    if(gtk_gl_area_get_error(area) != NULL)
    {
        dengineutils_logging_log("ERROR::Cannot make_current glarea");
        return;
    }else
    {
        int maj, min;
        gdk_gl_context_get_version( gtk_gl_area_get_context(area), &maj, &min);
        dengineutils_logging_log("INFO::GtkGLArea %d.%d realized", maj, min);
    }

    DengineInitOpts* opts = dengine_init_get_opts();
    opts->gl_loaddefault = 1;
    opts->window_createnative = 0;

    // preload glad to set viewport
    gladLoadGL();
    GtkAllocation* alloc = g_new(GtkAllocation, 1);
    gtk_widget_get_allocation(GTK_WIDGET(area), alloc);
    glViewport(0, 0, alloc->width, alloc->height);
    g_free(alloc);

    dengine_init();
}

void dengitor_scene_glarea_onunrealize(GtkGLArea* area)
{
    gtk_gl_area_make_current(area);
    if(gtk_gl_area_get_error(area) != NULL)
    {
        dengineutils_logging_log("ERROR::Cannot make_current glarea");
        return;
    }else
    {
        dengineutils_logging_log("INFO::GtkGLArea unrealized");
    }
}

void dengitor_scene_glarea_onrender(GtkGLArea* area)
{

}
