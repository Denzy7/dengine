#ifndef DENGITOR_INSPECTOR_H
#define DENGITOR_INSPECTOR_H

#include <gtk/gtk.h>
#include <dengine/dengine.h>

typedef struct
{
    GtkContainer* inspector;
    Entity* currententity;

    GtkWidget* transform_widget;
    GtkWidget* camera_widget;
    GtkWidget* light_widget;
    GtkWidget* mesh_widget;
}DengitorInspector;

/* for the container that parents  widget of type wtype,
 * set vector component v[i] of type vtype, 
 * increasing i each time a widget of wtype is found.
 * also redraws the whole window
 *
 * works for the following basic input widgets: 
 *  entry, color button, scale, check button
 */
guint dengitor_inspector_vecn_set(GtkWidget* widget, GType wtype, void* v, DengineType vtype, guint n);

/* same as dengitor_inspector_vecn_set but get value to put in widget from v[i]
 * a more appropriate name woulb be getfrom?
 *
 * doesn't redraw window as its unnecessary
 */
guint dengitor_inspector_vecn_get(GtkWidget* widget, GType wtype, void* v, DengineType vtype, guint n);

void dengitor_inspector_setup(GtkBuilder* builder, DengitorInspector* inspector);

void dengitor_inspector_do_entity(Entity* entity, DengitorInspector* inspector);

#endif // DENGITOR_INSPECTOR_H
