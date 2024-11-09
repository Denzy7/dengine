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
    GtkWidget* prefs;

    GtkComboBox* theme;
    PrefsTheme* themes;
    uint32_t themes_count;

    const char* breeze_dark;
    uint32_t breeze_dark_sz;
    const char* breeze_light;
    uint32_t breeze_light_sz;
    GtkCssProvider* provider;
}DengitorPrefs;

void dengitor_prefs_setup(GtkBuilder* builder, DengitorPrefs* prefs);


#endif // DENGITOR_PREFS_H
