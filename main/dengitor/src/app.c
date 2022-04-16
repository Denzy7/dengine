#include <gtk/gtk.h>

#include "dengitor/dengitor.h"

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
