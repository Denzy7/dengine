#include "dengitor/scenetree.h"

void _dengitor_scenetree_traverse_entity(GtkTreeIter* root, GtkTreeStore* store, Entity* entity);

void _dengitor_scenetree_onkeypress(GtkTreeView* tree, GdkEventKey* ev);

void _dengitor_scenetree_onrowactivated(GtkTreeView* tree);

void _dengitor_scenetree_onrowactivated(GtkTreeView* tree)
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

void _dengitor_scenetree_onkeypress(GtkTreeView* tree, GdkEventKey* ev)
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

void _dengitor_scenetree_traverse_entity(GtkTreeIter* root, GtkTreeStore* store, Entity* entity)
{
    GtkTreeIter iter_p, iter_c;
    //dengineutils_logging_log("trav %s", entity->name);

    if(!root)
    {
        //root
        gtk_tree_store_append(store, &iter_p, NULL);
        gtk_tree_store_set(store, &iter_p,
                           DENGITOR_SCENETREE_ENTNAME, entity->name,
                           DENGITOR_SCENETREE_ENTPTR, entity,
                           -1);
    }else
    {
        // TODO : potential leak??
        iter_p = *gtk_tree_iter_copy(root);
    }

    EntityChild* ec = entity->children.data;
    for(size_t i = 0; i < entity->children.count; i++)
    {
        Entity* child = ec[i].child;
        gtk_tree_store_append(store, &iter_c, &iter_p);
        gtk_tree_store_set(store, &iter_c,
                           DENGITOR_SCENETREE_ENTNAME, child->name,
                           DENGITOR_SCENETREE_ENTPTR, child,
                           -1);
        _dengitor_scenetree_traverse_entity(&iter_c, store, child);
    }
}

void dengitor_scenetree_traverse(Scene* scene, GtkTreeStore* store)
{
    gtk_tree_store_clear(store);
    EntityChild* ec = scene->entities.data;
    for(size_t i = 0; i < scene->entities.count; i++)
    {
       _dengitor_scenetree_traverse_entity(NULL, store, ec[i].child);
    }
}

void dengitor_scenetree_setup(GtkTreeView* tree)
{
    GtkTreeStore* store= gtk_tree_store_new(DENGITOR_SCENETREE_COLCNT,
                                           G_TYPE_STRING,
                                           G_TYPE_POINTER);
    gtk_tree_view_set_model(tree, GTK_TREE_MODEL(store));
    g_object_unref(store);

    GtkCellRenderer* renderer;

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(tree,
                                                -1, "Name",
                                                renderer, "text",
                                                DENGITOR_SCENETREE_ENTNAME, NULL);

    g_signal_connect(tree, "key-press-event", G_CALLBACK(_dengitor_scenetree_onkeypress), NULL);
    g_signal_connect(tree, "row-activated", G_CALLBACK(_dengitor_scenetree_onrowactivated), NULL);

}
