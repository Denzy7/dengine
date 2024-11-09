#include "dengitor/dengitor.h"

typedef enum
{
    WIDGET_OP_SET,
    WIDGET_OP_GET,
}WidgetOp;

typedef struct
{
    GtkWidget** widget;
    const char* str;
}ComponentMap;

typedef struct
{
    void* map;
    GtkWidget* widget;
}WidgetMap;

guint dengitor_inspector_vecn_op(GtkWidget* widget, GType wtype, void* v, DengineType vtype, guint n, WidgetOp op)
{
    GList* l = gtk_container_get_children(GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(widget))));
    char str[1024];
    guint i = 0, j = 0;
    for(i = 0; i < g_list_length(l); i++)
    {
        memset(str, 0, sizeof(str));

        GtkWidget* get = g_list_nth(l, i)->data;

        if(j > n)
            break;

        if(wtype == GTK_TYPE_ENTRY && GTK_IS_ENTRY(get))
        {
            if(widget == get)
            {
                if(op == WIDGET_OP_SET)
                {
                    if(!dengineutils_types_parse(vtype,
                                gtk_entry_get_text(GTK_ENTRY(get)),
                                v + (j * dengineutils_types_get_size(vtype))
                                )
                      )
                    {
                        memset(v + (j * dengineutils_types_get_size(vtype)), 0, dengineutils_types_get_size(vtype));
                    }
                }else if(op == WIDGET_OP_GET)
                {
                    if(dengineutils_types_tostr(vtype, str, sizeof(str), v + (j * dengineutils_types_get_size(vtype))))
                    {
                        GtkEntryBuffer* buffer = gtk_entry_get_buffer(GTK_ENTRY(get));
                        gtk_entry_buffer_set_text(buffer, str, strlen(str));
                    }
                }
            }else {
                j++;
            }
        }else if(wtype == GTK_TYPE_COLOR_BUTTON && GTK_IS_COLOR_BUTTON(get))
        {
            if(widget == get)
            {
                /*TODO: use like the scale example below. convert to str then parse.
                 * but are we going to incr j for every iteration of r,g,b,a?*/
                if(vtype != DENGINEUTILS_TYPE_FLOAT)
                    dengineutils_logging_log("WARNING::need float type for vecn");
                float* fv = v;
                GdkRGBA rgba;
                if(op == WIDGET_OP_SET)
                {
                    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(get), &rgba);

                    fv[0] = rgba.red;
                    if(n > 1)
                        fv[1] = rgba.green;
                    if(n > 2)
                        fv[2] = rgba.blue;
                    if(n > 3)
                        fv[3] = rgba.alpha;
                }else {
                    rgba.red = fv[0];
                    if(n > 1)
                        rgba.green = fv[1];
                    if(n > 2)
                        rgba.blue = fv[2];
                    if(n > 3)
                        rgba.alpha = fv[3];
                    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(get), &rgba);
                }
                break;
            }else {
                j++;
            }
        }else if(wtype == GTK_TYPE_SCALE && GTK_IS_SCALE(get))
        {
            if(widget == get)
            {
                double val;
                if(op == WIDGET_OP_SET)
                {
                    val = gtk_range_get_value(GTK_RANGE(get));
                    dengineutils_types_tostr(DENGINEUTILS_TYPE_DOUBLE, str, sizeof(str), &val);
                    dengineutils_types_parse(vtype,
                            str,
                            v + (j * dengineutils_types_get_size(vtype)));
                }else if (op == WIDGET_OP_GET) {
                    dengineutils_types_tostr(vtype, str, sizeof(str),
                            v + (j * dengineutils_types_get_size(vtype)));
                    dengineutils_types_parse(DENGINEUTILS_TYPE_DOUBLE,
                            str,
                            &val);
                    gtk_range_set_value(GTK_RANGE(get), val);
                }

            }else {
                j++;
            }
        }else if(wtype == GTK_TYPE_CHECK_BUTTON && GTK_IS_CHECK_BUTTON(get))
        {
            if(widget == get)
            {
                int val;
                if(op == WIDGET_OP_SET)
                {
                    val = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(get));
                    dengineutils_types_tostr(DENGINEUTILS_TYPE_INT32, str, sizeof(str), &val);
                    dengineutils_types_parse(vtype,
                            str,
                            v + (j * dengineutils_types_get_size(vtype)));
                }else if (op == WIDGET_OP_GET) {
                    dengineutils_types_tostr(vtype, str, sizeof(str),
                            v + (j * dengineutils_types_get_size(vtype)));
                    dengineutils_types_parse(vtype,
                            str,
                            &val);
                    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(get), val);
                }
            }else {
                j++;
            }
        }
    }
    if(op == WIDGET_OP_SET)
        dengitor_redraw();
    return j;
}
guint dengitor_inspector_vecn_set(GtkWidget* widget, GType wtype, void* v, DengineType vtype, guint n)
{
    return dengitor_inspector_vecn_op(widget, wtype, v, vtype, n, WIDGET_OP_SET);
}
guint dengitor_inspector_vecn_get(GtkWidget* widget, GType wtype, void* v, DengineType vtype, guint n)
{
    return dengitor_inspector_vecn_op(widget, wtype, v, vtype, n, WIDGET_OP_GET);
}



void dengitor_inspector_setup(GtkBuilder* builder, DengitorInspector* inspector)
{
    inspector->inspector = GTK_CONTAINER(gtk_builder_get_object(builder, "inspector_vbox"));

    ComponentMap components[] = 
    {
        {&inspector->transform_widget, "transform_component"},
        {&inspector->camera_widget, "camera_component"},
        {&inspector->light_widget, "light_component"},
        {&inspector->mesh_widget, "mesh_component"},
    };

    GtkContainer* container;
    for(size_t i = 0; i < DENGINE_ARY_SZ(components); i++)
    {
        *components[i].widget = GTK_WIDGET( gtk_builder_get_object(builder, components[i].str));
        container = GTK_CONTAINER( gtk_widget_get_parent(*components[i].widget) );
        dengitor_utils_changecontainer(*components[i].widget,
                container, inspector->inspector);
        g_object_unref(container);
    }
}

void dengitor_inspector_do_entity(Entity* entity, DengitorInspector* inspector)
{
    // hide all. only show if we have the component
    GList* l = gtk_container_get_children(GTK_CONTAINER(inspector->inspector));
    for(size_t i = 0; i < g_list_length(l); i++)
    {
        GtkWidget* w = g_list_nth(l, i)->data;
        gtk_widget_hide(w);
    }

    if(!entity)
        return;

    inspector->currententity = entity;

    WidgetMap map[] = 
    {
        {entity->mesh_component, inspector->mesh_widget},
        {entity->light_component, inspector->light_widget},
        {entity->camera_component, inspector->camera_widget},
    };

    gtk_widget_show_all(inspector->transform_widget);
    /*show conditionally so widgets dont have to check for component 
     */

    for(size_t i = 0; i < DENGINE_ARY_SZ(map); i++)
    {
        if(map[i].map)
            gtk_widget_show_all(map[i].widget);
    }
    //pull transform data
}
