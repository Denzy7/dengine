#include "dengitor/dengitor.h"

DENGINE_EXPORT void dengitor_inspector_component_light_type_onmap(GtkComboBox* cbx)
{
    Entity* current = dengitor_get()->inspector.currententity;
    if(!current)
        return;

    gtk_combo_box_set_active(cbx, current->light_component->type);
        
}
DENGINE_EXPORT void dengitor_inspector_component_light_type_onchange(GtkComboBox* cbx)
{
}

DENGINE_EXPORT void dengitor_inspector_component_light_ambient_onmap(GtkColorButton* btn)
{
    Entity* current = dengitor_get()->inspector.currententity;
    if(!current)
        return;

    LightOp* lightop = dengine_lighting_light_get_lightop(current->light_component->type, current->light_component->light);
    dengitor_inspector_vecn_get(GTK_WIDGET(btn), GTK_TYPE_COLOR_BUTTON, lightop->ambient, DENGINEUTILS_TYPE_FLOAT, 4);
}
DENGINE_EXPORT void dengitor_inspector_component_light_ambient_onchange(GtkColorButton* btn)
{
    Entity* current = dengitor_get()->inspector.currententity;
    LightOp* lightop = dengine_lighting_light_get_lightop(current->light_component->type, current->light_component->light);
    dengitor_inspector_vecn_set(GTK_WIDGET(btn), GTK_TYPE_COLOR_BUTTON, lightop->ambient, DENGINEUTILS_TYPE_FLOAT, 4);
}


DENGINE_EXPORT void dengitor_inspector_component_light_diffuse_onmap(GtkColorButton* btn)
{
    Entity* current = dengitor_get()->inspector.currententity;
    if(!current)
        return;

    LightOp* lightop = dengine_lighting_light_get_lightop(current->light_component->type, current->light_component->light);
    dengitor_inspector_vecn_get(GTK_WIDGET(btn), GTK_TYPE_COLOR_BUTTON, lightop->diffuse, DENGINEUTILS_TYPE_FLOAT, 4);
}

DENGINE_EXPORT void dengitor_inspector_component_light_diffuse_onchange(GtkColorButton* btn)
{
    Entity* current = dengitor_get()->inspector.currententity;
    if(!current)
        return;

    LightOp* lightop = dengine_lighting_light_get_lightop(current->light_component->type, current->light_component->light);
    dengitor_inspector_vecn_set(GTK_WIDGET(btn), GTK_TYPE_COLOR_BUTTON, lightop->diffuse, DENGINEUTILS_TYPE_FLOAT, 4);
}

DENGINE_EXPORT void dengitor_inspector_component_light_strength_onmap(GtkScale* scl)
{
    Entity* current = dengitor_get()->inspector.currententity;
    if(!current)
        return;

    LightOp* lightop = dengine_lighting_light_get_lightop(current->light_component->type, current->light_component->light);
    dengitor_inspector_vecn_get(GTK_WIDGET(scl), GTK_TYPE_SCALE, &lightop->strength, DENGINEUTILS_TYPE_FLOAT, 1);
}

DENGINE_EXPORT void dengitor_inspector_component_light_strength_onchange(GtkScale* scl)
{
    Entity* current = dengitor_get()->inspector.currententity;
    LightOp* lightop = dengine_lighting_light_get_lightop(current->light_component->type, current->light_component->light);
    dengitor_inspector_vecn_set(GTK_WIDGET(scl), GTK_TYPE_SCALE, &lightop->strength, DENGINEUTILS_TYPE_FLOAT, 1);
}

DENGINE_EXPORT void dengitor_inspector_component_light_shadow_toggle_ontoggle(GtkCheckButton* btn)
{
    Entity* current = dengitor_get()->inspector.currententity;
    ShadowOp* shadowop = dengine_lighting_light_get_shadowop(current->light_component->type, current->light_component->light);
    dengitor_inspector_vecn_set(GTK_WIDGET(btn), GTK_TYPE_CHECK_BUTTON, 
            &shadowop->enable, DENGINEUTILS_TYPE_INT32, 1);
}
DENGINE_EXPORT void dengitor_inspector_component_light_shadow_toggle_onmap(GtkCheckButton* btn)
{
    Entity* current = dengitor_get()->inspector.currententity;
    if(!current)
        return;
    ShadowOp* shadowop = dengine_lighting_light_get_shadowop(current->light_component->type, current->light_component->light);
    dengitor_inspector_vecn_get(GTK_WIDGET(btn), GTK_TYPE_CHECK_BUTTON, 
            &shadowop->enable, DENGINEUTILS_TYPE_INT32, 1);
}
