#include <dengine/dengine.h>
#include "dengitor/dengitor.h"

int main(int argc, char *argv[])
{

    GtkApplication* app = gtk_application_new(
                "com.denzygames.Dengitor", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(dengitor_app_onactivate), NULL);
    int run = g_application_run( G_APPLICATION(app), argc, argv);
    return run;
}

void dengitor_app_onactivate(GtkApplication* app)
{
    GtkWidget* window = gtk_application_window_new(app);
    gtk_window_set_default_size( GTK_WINDOW(window), 1280, 720);
    gtk_window_set_title( GTK_WINDOW(window), "Dengitor");

    GtkWidget* about = gtk_button_new_with_label("WORK IN PROGRESS...");
    g_signal_connect(about, "clicked", G_CALLBACK(dengitor_aboutdialog), NULL);
    gtk_container_add(GTK_CONTAINER(window), about);

    gtk_widget_show_all(window);
}

void dengitor_aboutdialog()
{
    GtkAboutDialog* about = GTK_ABOUT_DIALOG(gtk_about_dialog_new());
    gtk_about_dialog_set_program_name(about, "Dengitor");
    gtk_widget_show_all(GTK_WIDGET(about));
    gtk_about_dialog_set_comments(about, "Dengine Editor. Powered by GTK+ 3");
    gtk_about_dialog_set_version(about, DENGINE_VERSION);
    gtk_about_dialog_set_logo_icon_name(about, "input-gaming");
}

