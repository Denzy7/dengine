#include <dengine/dengine.h>
#include <gtk/gtk.h>

#include "dengitor/dengitor.h"
#include "dengitor/app.h"

int main(int argc, char *argv[])
{

    GtkApplication* app = gtk_application_new(
                "com.denzygames.Dengitor", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(dengitor_app_onactivate), NULL);
    int run = g_application_run( G_APPLICATION(app), argc, argv);
    return run;
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

