#ifndef DENGITOR_H
#define DENGITOR_H

#include <gtk/gtk.h>

typedef struct
{
    GtkWidget* main;
    GtkAboutDialog* about;
    GtkBuilder* builder;
}Dengitor;

void dengitor_onactivate(GtkApplication* app);

void dengitor_aboutdialog();

#endif // DENGITOR_H
