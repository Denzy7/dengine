#include "dengitor/dengitor.h"

DENGINE_EXPORT void _dengitor_scenetree_onrowactivated(GtkTreeView* tree)
{
    GtkTreeModel* model = gtk_tree_view_get_model(tree);
    GtkTreeSelection* selection = gtk_tree_view_get_selection(tree);
    GtkTreeIter iter;
    if(gtk_tree_selection_get_selected(selection, &model,&iter))
    {
        GtkTreePath* path = gtk_tree_model_get_path(model, &iter);
        gtk_tree_view_expand_row(tree, path, false);
    }
}

DENGINE_EXPORT void _dengitor_scenetree_onkeypress(GtkTreeView* tree, GdkEventKey* ev)
{
    GtkTreeModel* model = gtk_tree_view_get_model(tree);
    GtkTreeSelection* selection = gtk_tree_view_get_selection(tree);
    GtkTreeIter iter;
    if(gtk_tree_selection_get_selected(selection, &model,&iter))
    {
        GtkTreePath* path = gtk_tree_model_get_path(model, &iter);

        if(ev->keyval == GDK_KEY_Left)
            gtk_tree_view_collapse_row(tree, path);
        else if(ev->keyval == GDK_KEY_Right)
            gtk_tree_view_expand_row(tree, path, false);
    }
}

void _dengitor_scenetree_traverse_entity(GtkTreeStore* store, Entity* entity, GtkTreeIter* iter_p)
{
    GtkTreeIter iter_c;

    gtk_tree_store_append(store, &iter_c, iter_p);
    gtk_tree_store_set(store, &iter_c,
            DENGITOR_SCENETREE_ENTNAME, entity->name,
            DENGITOR_SCENETREE_ENTPTR, entity,
            -1);

    Entity** ch = entity->children.data;
    for(size_t i = 0; i < entity->children.count; i++)
    {
        _dengitor_scenetree_traverse_entity(store, ch[i], &iter_c);
    }
    //dengineutils_logging_log("trav %s", entity->name);
}

void dengitor_scenetree_traverse(Scene* scene, DengitorSceneTree* tree)
{
    GtkTreeStore* store = GTK_TREE_STORE(gtk_tree_view_get_model(tree->tree));
    gtk_tree_store_clear(store);
    Entity** e = scene->entities.data;
    for(size_t i = 0; i < scene->entities.count; i++)
    {
       _dengitor_scenetree_traverse_entity(store, e[i], NULL);
    }
}



DENGINE_EXPORT void dengitor_scene_treeview_oncursorchange(GtkTreeView* tree)
{
    Dengitor* dengitor = dengitor_get(); 
    GtkTreeModel* model = gtk_tree_view_get_model(tree);
    GtkTreeSelection* selection = gtk_tree_view_get_selection(tree);
    GtkTreeIter iter;
    if(gtk_tree_selection_get_selected(selection, &model,&iter))
    {
        char* name;
        gtk_tree_model_get(model, &iter,
                           DENGITOR_SCENETREE_ENTNAME, &name, -1);
        Entity* current = NULL;
        gtk_tree_model_get(model, &iter,
                           DENGITOR_SCENETREE_ENTPTR, &current, -1);
        dengineutils_logging_log("selected %s %p %u", name, current, current->entity_id);

        dengitor_inspector_do_entity(current, &dengitor->inspector);

        free(name);
    }
}

void dengitor_scenetree_setup(GtkBuilder* builder, DengitorSceneTree* tree)
{
    tree->tree = GTK_TREE_VIEW(gtk_builder_get_object(builder, "scene_treeview"));
    GtkTreeStore* store= gtk_tree_store_new(DENGITOR_SCENETREE_COLCNT,
                                           G_TYPE_STRING,
                                           G_TYPE_POINTER);
    gtk_tree_view_set_model(tree->tree, GTK_TREE_MODEL(store));
    g_object_unref(store);

    GtkCellRenderer* renderer;

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(tree->tree,
                                                -1, "Name",
                                                renderer, "text",
                                                DENGITOR_SCENETREE_ENTNAME, NULL);

}
