#ifndef DENGITOR_H
#define DENGITOR_H

#include <gtk/gtk.h>

typedef struct
{
    GtkApplicationWindow* main;
    GtkAboutDialog* about;
    GtkBuilder* builder;
}Dengitor;

void dengitor_onactivate(GtkApplication* app);

void dengitor_aboutdialog_show();

void dengitor_aboutdialog_hide();
#endif // DENGITOR_H
