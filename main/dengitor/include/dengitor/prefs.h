#ifndef DENGITOR_PREFS_H
#define DENGITOR_PREFS_H

#include <gtk/gtk.h>

typedef struct
{
    char* css;
    uint32_t length;
}PrefsTheme;

typedef struct
{
    GtkBuilder* builder;
    GtkWidget* prefs;

    GtkComboBox* theme;
    PrefsTheme* themes;
    uint32_t themes_count;

    GtkCssProvider* provider;

    GtkFontChooser* font;
    GtkButton* font_change;

    GtkButton* ok;
    GtkButton* cancel;

}Prefs;

void dengitor_prefs_setup(GtkBuilder* builder, Prefs* prefs);

void dengitor_prefs_show(GtkWidget* widget, Prefs* prefs);

void dengitor_prefs_hide(GtkWidget* widget, Prefs* prefs);

#endif // DENGITOR_PREFS_H
