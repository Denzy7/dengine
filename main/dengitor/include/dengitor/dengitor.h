#ifndef DENGITOR_H
#define DENGITOR_H

#include <gtk/gtk.h>

typedef struct
{
    GtkWidget* main;
    GtkAboutDialog* about;
    GtkBuilder* builder;
}Dengitor;

static Dengitor dengitor;

void dengitor_aboutdialog();

#endif // DENGITOR_H
