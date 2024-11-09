#include "dengitor/dengitor.h"

typedef struct
{
    const char* css;
    uint32_t sz;
}DengitorTheme;

DENGINE_EXPORT void _dengitor_prefs_theme(GtkComboBox* combo)
{
    DengitorPrefs* prefs = &dengitor_get()->prefs;
    DengitorTheme themes[] = 
    {
        {NULL, 0},
        {prefs->breeze_dark, prefs->breeze_dark_sz},
        {prefs->breeze_light, prefs->breeze_light_sz},
    };
    guint id = gtk_combo_box_get_active(combo);

    GdkScreen* screen = gdk_display_get_default_screen( gdk_display_get_default() );
    if(themes[id].css)
    {
        gtk_css_provider_load_from_data(prefs->provider, 
                themes[id].css, themes[id].sz, NULL);
        gtk_style_context_add_provider_for_screen(screen,
                                                  GTK_STYLE_PROVIDER(prefs->provider),
                                                  GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }else
    {
        gtk_style_context_remove_provider_for_screen(screen,
                                                     GTK_STYLE_PROVIDER(prefs->provider));
    } 
    dengitor_redraw();
}

DENGINE_EXPORT void _dengitor_prefs_fontset(GtkFontButton* btn)
{
    GtkFontChooser* font_chooser = GTK_FONT_CHOOSER(btn);
    const char* font = gtk_font_chooser_get_font(font_chooser);
    GtkSettings* settings = gtk_settings_get_for_screen(gdk_screen_get_default());
    g_object_set(settings, "gtk-font-name", font, NULL);
    dengitor_redraw();
}

void dengitor_prefs_setup(GtkBuilder* builder, DengitorPrefs* prefs)
{
    prefs->provider = gtk_css_provider_new();
}


