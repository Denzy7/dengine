#ifndef DENGITOR_PREFS_H
#define DENGITOR_PREFS_H

#include <gtk/gtk.h>
typedef struct
{
    GtkBuilder* builder;
    GtkWidget* prefs;

    GtkComboBox* theme;
    GtkCssProvider* provider;

    GtkFontChooser* font;

    GtkButton* ok;
    GtkButton* cancel;

}Prefs;

void dengitor_prefs_setup(GtkBuilder* builder, Prefs* prefs);

void dengitor_prefs_show(GtkWidget* widget, Prefs* prefs);

void dengitor_prefs_hide(GtkWidget* widget, Prefs* prefs);

#endif // DENGITOR_PREFS_H
