#include "dengitor/utils.h"

void dengitor_utils_changecontainer(GtkWidget* widget, GtkContainer* container_old, GtkContainer* container_new)
{
    g_object_ref(widget);
    gtk_container_remove(container_old, widget);
    gtk_container_add(container_new, widget);
    g_object_unref(widget);
}

gulong dengitor_utils_get_signal(GType type, gpointer instance, const char* name)
{
    guint id;
    GQuark detail;
    g_signal_parse_name(name, type, &id, &detail, FALSE);
    return g_signal_handler_find(instance, G_SIGNAL_MATCH_ID, id, detail, NULL, NULL, NULL);
}

void dengitor_utils_disconnect(GType type, gpointer instance, const char* signal)
{
    gulong sig = dengitor_utils_get_signal(type, instance, signal);
    if(sig)
        g_signal_handler_disconnect(instance, sig);
}

void dengitor_utils_rgba_to_float4(GdkRGBA* rgba, float* float4)
{
    float4[0] = rgba->red;
    float4[1] = rgba->green;
    float4[2] = rgba->blue;
    float4[3] = rgba->alpha;
}

void dengitor_utils_float4_to_rgba(float* float4, GdkRGBA* rgba)
{
    rgba->red = float4[0];
    rgba->green = float4[1];
    rgba->blue = float4[2];
    rgba->alpha = float4[3];
}
