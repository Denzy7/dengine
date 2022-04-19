#include "dengitor/prefs.h"

void _dengitor_prefs_ok()
{

}

void dengitor_prefs_setup(GtkBuilder* builder, Prefs* prefs)
{
    prefs->prefs = GTK_WIDGET( gtk_builder_get_object(builder, "prefs") );
    prefs->theme = GTK_COMBO_BOX( gtk_builder_get_object(builder, "theme") );

    prefs->ok = GTK_BUTTON( gtk_builder_get_object(builder, "ok") );
    prefs->cancel= GTK_BUTTON( gtk_builder_get_object(builder, "cancel") );

    prefs->builder = builder;
}

void dengitor_prefs_show(Prefs* prefs)
{
    gtk_widget_show_all(prefs->prefs);
}
