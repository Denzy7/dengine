#include "dengitor/dengitor.h"

DENGINE_EXPORT void _dengitor_inspector_component_transform_pos_onchange(GtkEntry* entry)
{
    Entity* current = dengitor_get()->inspector.currententity;
    dengitor_inspector_vecn_set(GTK_WIDGET(entry), GTK_TYPE_ENTRY, 
            current->transform.position, DENGINEUTILS_TYPE_FLOAT,
            3);
}

DENGINE_EXPORT void _dengitor_inspector_component_transform_pos_onmap(GtkEntry* entry)
{
    Entity* current = dengitor_get()->inspector.currententity;
    if(!current)
        return;
    dengitor_inspector_vecn_get(GTK_WIDGET(entry), GTK_TYPE_ENTRY, 
            current->transform.position, DENGINEUTILS_TYPE_FLOAT,
            3);
}
DENGINE_EXPORT void _dengitor_inspector_component_transform_rot_onchange(GtkEntry* entry)
{
    Entity* current = dengitor_get()->inspector.currententity;
    dengitor_inspector_vecn_set(GTK_WIDGET(entry), GTK_TYPE_ENTRY, 
            current->transform.rotation, DENGINEUTILS_TYPE_FLOAT,
            3);
}

DENGINE_EXPORT void _dengitor_inspector_component_transform_rot_onmap(GtkEntry* entry)
{
    Entity* current = dengitor_get()->inspector.currententity;
    if(!current)
        return;
    dengitor_inspector_vecn_get(GTK_WIDGET(entry), GTK_TYPE_ENTRY, 
            current->transform.rotation, DENGINEUTILS_TYPE_FLOAT,
            3);
}
