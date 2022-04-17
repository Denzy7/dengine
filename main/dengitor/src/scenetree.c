#include "dengitor/scenetree.h"

enum
{
    DENGITOR_SCENETREE_ENTNAME = 0,
    DENGITOR_SCENETREE_ENTPTR,
    DENGITOR_SCENETREE_COLCNT
};


void _dengitor_scenetree_traverse_entity(GtkTreeIter* root, GtkTreeStore* store, Entity* entity);

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

    for(size_t i = 0; i < entity->children_count; i++)
    {
        Entity* child = entity->children[i];
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
    for(size_t i = 0; i < scene->n_entities; i++)
    {
       _dengitor_scenetree_traverse_entity(NULL, store, scene->entities[i]);
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
}
