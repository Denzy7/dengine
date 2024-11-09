#include "dengitor/dengitor.h"

DENGINE_EXPORT void dengitor_viewportopts_grid_draw_onmap(GtkCheckButton* btn)
{
    Dengitor* dengitor = dengitor_get();
    dengitor_inspector_vecn_get(GTK_WIDGET(btn), GTK_TYPE_CHECK_BUTTON, &dengitor->scene_grid_draw, DENGINEUTILS_TYPE_INT32, 1);
}
DENGINE_EXPORT void dengitor_viewportopts_grid_draw_ontoggle(GtkCheckButton* btn)
{
    Dengitor* dengitor = dengitor_get();
    dengitor_inspector_vecn_set(GTK_WIDGET(btn), GTK_TYPE_CHECK_BUTTON, &dengitor->scene_grid_draw, DENGINEUTILS_TYPE_INT32, 1);
}

DENGINE_EXPORT void dengitor_viewportopts_grid_color_onmap(GtkColorButton* btn)
{
    Dengitor* dengitor = dengitor_get();
    dengitor_inspector_vecn_get(GTK_WIDGET(btn), GTK_TYPE_COLOR_BUTTON, dengitor->scene_grid_color, DENGINEUTILS_TYPE_FLOAT, 4);
}
DENGINE_EXPORT void dengitor_viewportopts_grid_color_oncolorset(GtkColorButton* btn)
{
    Dengitor* dengitor = dengitor_get();
    dengitor_inspector_vecn_set(GTK_WIDGET(btn), GTK_TYPE_COLOR_BUTTON, dengitor->scene_grid_color, DENGINEUTILS_TYPE_FLOAT, 4);
}

DENGINE_EXPORT void dengitor_viewportopts_grid_scale_onmap(GtkScale* scl)
{
    Dengitor* dengitor = dengitor_get();
    dengitor_inspector_vecn_get(GTK_WIDGET(scl), GTK_TYPE_SCALE, &dengitor->scene_grid_scale, DENGINEUTILS_TYPE_FLOAT, 1);
}
DENGINE_EXPORT void dengitor_viewportopts_grid_scale_onvaluechange(GtkScale* scl)
{
    Dengitor* dengitor = dengitor_get();
    dengitor_inspector_vecn_set(GTK_WIDGET(scl), GTK_TYPE_SCALE, &dengitor->scene_grid_scale, DENGINEUTILS_TYPE_FLOAT, 1);
}

DENGINE_EXPORT void dengitor_viewportopts_grid_width_onmap(GtkScale* scl)
{
    Dengitor* dengitor = dengitor_get();
    dengitor_inspector_vecn_get(GTK_WIDGET(scl), GTK_TYPE_SCALE, &dengitor->scene_grid_width, DENGINEUTILS_TYPE_FLOAT, 1);
}
DENGINE_EXPORT void dengitor_viewportopts_grid_width_onvaluechange(GtkScale* scl)
{
    Dengitor* dengitor = dengitor_get();
    dengitor_inspector_vecn_set(GTK_WIDGET(scl), GTK_TYPE_SCALE, &dengitor->scene_grid_width, DENGINEUTILS_TYPE_FLOAT, 1);
}

DENGINE_EXPORT void dengitor_viewportopts_fov_onmap(GtkScale* scl)
{
    Dengitor* dengitor = dengitor_get();
    dengitor_inspector_vecn_get(GTK_WIDGET(scl), GTK_TYPE_SCALE, &dengitor->scene_camera->camera_component->camera.fov, DENGINEUTILS_TYPE_FLOAT, 1);
}
DENGINE_EXPORT void dengitor_viewportopts_fov_onvaluechange(GtkScale* scl)
{
    Dengitor* dengitor = dengitor_get();
    dengitor_inspector_vecn_set(GTK_WIDGET(scl), GTK_TYPE_SCALE, &dengitor->scene_camera->camera_component->camera.fov, DENGINEUTILS_TYPE_FLOAT, 1);
}

void dengitor_viewportopts_setup(GtkBuilder* builder, DengitorViewportOpts* viewportopts)
{
    viewportopts->viewport_opts_dialog = GTK_DIALOG( gtk_builder_get_object(builder, "viewport_opts") );
}

DENGINE_EXPORT void dengitor_viewportopts_padding_onmap(GtkScale* scl)
{
    Dengitor* dengitor = dengitor_get();
    dengitor_inspector_vecn_get(GTK_WIDGET(scl), GTK_TYPE_SCALE, &dengitor->viewportops.viewport_padding, DENGINEUTILS_TYPE_FLOAT, 1);
}
DENGINE_EXPORT void dengitor_viewportopts_padding_onchange(GtkScale* scl)
{
    Dengitor* dengitor = dengitor_get();
    dengitor_inspector_vecn_set(GTK_WIDGET(scl), GTK_TYPE_SCALE, &dengitor->viewportops.viewport_padding, DENGINEUTILS_TYPE_FLOAT, 1);
}
