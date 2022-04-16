#include <gtk/gtk.h>

#include "dengitor/dengitor.h"

void dengitor_app_onactivate(GtkApplication* app)
{
    //TODO : compile resources
    const char* ui = "/home/denzy/dengine/main/dengitor/res/default/dengine-editor-ui.glade";

    dengitor.builder = gtk_builder_new_from_file(ui);
    dengitor.main = GTK_WIDGET(gtk_builder_get_object(dengitor.builder, "main"));
    dengitor.about = GTK_WIDGET(gtk_builder_get_object(dengitor.builder, "about"));

    gtk_application_add_window(app, GTK_WINDOW(dengitor.main));

    gtk_widget_show_all(dengitor.main);
}
