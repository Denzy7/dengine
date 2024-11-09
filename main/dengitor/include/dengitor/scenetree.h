#ifndef SCENETREE_H
#define SCENETREE_H

#include <gtk/gtk.h>
#include <dengine/dengine.h>

enum
{
    DENGITOR_SCENETREE_ENTNAME = 0,
    DENGITOR_SCENETREE_ENTPTR,
    DENGITOR_SCENETREE_COLCNT
};

typedef struct
{
    GtkTreeView* tree;
}DengitorSceneTree;

void dengitor_scenetree_traverse(Scene* scene, DengitorSceneTree* tree);

void dengitor_scenetree_setup(GtkBuilder* builder, DengitorSceneTree* tree);

#endif // SCENETREE_H
