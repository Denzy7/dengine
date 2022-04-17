#include "dengitor/utils.h"

void dengitor_utils_changecontainer(GtkWidget* widget, GtkContainer* container_old, GtkContainer* container_new)
{
    g_object_ref(widget);
    gtk_container_remove(container_old, widget);
    gtk_container_add(container_new, widget);
    g_object_unref(widget);
}
