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

    gtk_application_add_window(app, GTK_WINDOW(dengitor.main));
    gtk_widget_show_all(GTK_WIDGET(dengitor.main));

    // menu button
    g_signal_connect(gtk_builder_get_object(dengitor.builder, "menu_action"),
                     "activate", G_CALLBACK(dengitor_aboutdialog_show), NULL);
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
