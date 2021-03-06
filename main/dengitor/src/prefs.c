#include "dengitor/prefs.h"
#include "dengine-utils/filesys.h"

void _dengitor_prefs_ok(gpointer ptr, Prefs* prefs)
{
    gtk_widget_hide(prefs->prefs);
}

void _dengitor_prefs_cancel(gpointer ptr, Prefs* prefs)
{
    gtk_widget_hide(prefs->prefs);
}

void _dengitor_prefs_theme(GtkComboBox* combo, Prefs* prefs)
{
    static const char* themes[]=
    {
        NULL,
        "breeze/dark.css",
        "breeze/light.css",
    };

    guint id = gtk_combo_box_get_active(combo);
    const char* style = themes[id];

    GdkScreen* screen = gdk_display_get_default_screen( gdk_display_get_default() );
    if(style)
    {
        char prtbf[1024];
        g_snprintf(prtbf, sizeof(prtbf), "%s/themes/%s", dengineutils_filesys_get_filesdir_dengine(), style);

        GFile* f = g_file_new_for_path(prtbf);
        gtk_css_provider_load_from_file(prefs->provider, f, NULL);

        gtk_style_context_add_provider_for_screen(screen,
                                                  GTK_STYLE_PROVIDER(prefs->provider),
                                                  GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }else
    {
        gtk_style_context_remove_provider_for_screen(screen,
                                                     GTK_STYLE_PROVIDER(prefs->provider));
    }
}

void _dengitor_prefs_fontset(GtkFontChooser* font_chooser, Prefs* prefs)
{
    const char* font = gtk_font_chooser_get_font(font_chooser);
    gtk_widget_hide( GTK_WIDGET(font_chooser) );

    GtkSettings* settings = gtk_settings_get_for_screen(gdk_screen_get_default());
    g_object_set(settings, "gtk-font-name", font, NULL);
}

void dengitor_prefs_setup(GtkBuilder* builder, Prefs* prefs)
{
    prefs->prefs = GTK_WIDGET( gtk_builder_get_object(builder, "prefs") );
    prefs->theme = GTK_COMBO_BOX( gtk_builder_get_object(builder, "theme") );
    g_signal_connect(prefs->theme, "changed", G_CALLBACK(_dengitor_prefs_theme), prefs);
    prefs->provider = gtk_css_provider_new();

    prefs->ok = GTK_BUTTON( gtk_builder_get_object(builder, "ok") );
    g_signal_connect(prefs->ok, "clicked", G_CALLBACK(_dengitor_prefs_ok), prefs);

    prefs->cancel= GTK_BUTTON( gtk_builder_get_object(builder, "cancel") );
    g_signal_connect(prefs->cancel, "clicked", G_CALLBACK(_dengitor_prefs_cancel), prefs);

    prefs->font = GTK_FONT_CHOOSER( gtk_builder_get_object(builder, "font") );
    prefs->font_change = GTK_BUTTON( gtk_builder_get_object(builder, "font_change") );

    g_signal_connect_swapped(
                prefs->font_change,
                "clicked", G_CALLBACK(gtk_widget_show_all), prefs->font);
    g_signal_connect(prefs->font, "response", G_CALLBACK(_dengitor_prefs_fontset), prefs);


    prefs->builder = builder;
}

void dengitor_prefs_show(GtkWidget* widget, Prefs* prefs)
{
    gtk_widget_show_all(prefs->prefs);

    char* font;
    g_object_get(gtk_settings_get_default(), "gtk-font-name", &font, NULL);

    gtk_button_set_label(prefs->font_change, font);

    g_free(font);
}


