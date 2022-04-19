#include "dengitor/prefs.h"

void _dengitor_prefs_ok(gpointer ptr, Prefs* prefs)
{
    gtk_widget_hide(prefs->prefs);
}

void _dengitor_prefs_cancel(gpointer ptr, Prefs* prefs)
{
    gtk_widget_hide(prefs->prefs);
}

void _dengitor_prefs_theme(GtkComboBox* combo)
{
    g_print("%d\n", gtk_combo_box_get_active(combo));
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


