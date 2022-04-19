#include "dengitor/prefs.h"

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
        g_snprintf(prtbf, sizeof(prtbf), "/com/denzygames/Dengitor/%s", style);

        prefs->provider = gtk_css_provider_new();
        gtk_css_provider_load_from_resource(prefs->provider, prtbf);

        gtk_style_context_add_provider_for_screen(screen,
                                                  GTK_STYLE_PROVIDER(prefs->provider),
                                                  GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }else
    {
        if(prefs->provider)
        {
            gtk_style_context_remove_provider_for_screen(screen,
                                                         GTK_STYLE_PROVIDER(prefs->provider));
        }
    }
}

void dengitor_prefs_setup(GtkBuilder* builder, Prefs* prefs)
{
    prefs->prefs = GTK_WIDGET( gtk_builder_get_object(builder, "prefs") );
    prefs->theme = GTK_COMBO_BOX( gtk_builder_get_object(builder, "theme") );
    g_signal_connect(prefs->theme, "changed", G_CALLBACK(_dengitor_prefs_theme), NULL);

    prefs->ok = GTK_BUTTON( gtk_builder_get_object(builder, "ok") );
    g_signal_connect(prefs->ok, "clicked", G_CALLBACK(_dengitor_prefs_ok), prefs);

    prefs->cancel= GTK_BUTTON( gtk_builder_get_object(builder, "cancel") );
    g_signal_connect(prefs->cancel, "clicked", G_CALLBACK(_dengitor_prefs_cancel), prefs);

    prefs->builder = builder;
}

void dengitor_prefs_show(GtkWidget* widget, Prefs* prefs)
{
    gtk_widget_show_all(prefs->prefs);
}


