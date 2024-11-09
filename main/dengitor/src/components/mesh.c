#include "dengitor/dengitor.h"

DENGINE_EXPORT void dengitor_inspector_components_mesh_mode_onmap(GtkComboBox* cbx)
{
    Dengitor* dengitor = dengitor_get();
    if(!dengitor->inspector.currententity)
        return;;

    gtk_combo_box_set_active(cbx, dengitor->inspector.currententity->mesh_component->drawmode);
}

DENGINE_EXPORT void dengitor_inspector_components_mesh_mode_onchange(GtkComboBox* cbx)
{
    dengitor_get()->inspector.currententity->mesh_component->drawmode = 
        gtk_combo_box_get_active(cbx);
    dengitor_redraw();
}
