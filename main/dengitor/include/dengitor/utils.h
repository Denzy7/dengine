#ifndef DENGITOR_UTILS_H
#define DENGITOR_UTILS_H

#include <gtk/gtk.h>

void dengitor_utils_changecontainer(GtkWidget* widget, GtkContainer* container_old, GtkContainer* container_new);

gulong dengitor_utils_get_signal(GType type, gpointer instance, const char* name);

void dengitor_utils_disconnect(GType type, gpointer instance, const char* signal);

#endif // UTILS_H
