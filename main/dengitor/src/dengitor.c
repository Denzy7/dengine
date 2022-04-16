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
    gtk_widget_show_all(window);
}
