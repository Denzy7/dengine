#include "dengitor/dengitor.h"

DENGINE_EXPORT void _dengine_inspector_component_camera_nearclip_onchange(GtkEntry* entry)
{
    Entity* current = dengitor_get()->inspector.currententity;
    dengitor_inspector_vecn_set(GTK_WIDGET(entry), GTK_TYPE_ENTRY,  &current->camera_component->camera.near, DENGINEUTILS_TYPE_FLOAT, 1);
}
DENGINE_EXPORT void _dengine_inspector_component_camera_nearclip_onmap(GtkEntry* entry)
{
    Entity* current = dengitor_get()->inspector.currententity;
    if(!current)
        return;
    dengitor_inspector_vecn_get(GTK_WIDGET(entry), GTK_TYPE_ENTRY,  &current->camera_component->camera.near, DENGINEUTILS_TYPE_FLOAT, 1);
}

DENGINE_EXPORT void _dengitor_inspector_component_camera_fov_onmap(GtkScale* scl)
{
    Entity* current = dengitor_get()->inspector.currententity;
    if(!current)
        return;
    dengitor_inspector_vecn_get(GTK_WIDGET(scl), GTK_TYPE_SCALE,  &current->camera_component->camera.fov, DENGINEUTILS_TYPE_FLOAT, 1);
}

DENGINE_EXPORT void _dengitor_inspector_component_camera_fov_onchange(GtkScale* scl)
{
    Entity* current = dengitor_get()->inspector.currententity;
    dengitor_inspector_vecn_set(GTK_WIDGET(scl), GTK_TYPE_SCALE,  &current->camera_component->camera.fov, DENGINEUTILS_TYPE_FLOAT, 1);
}
