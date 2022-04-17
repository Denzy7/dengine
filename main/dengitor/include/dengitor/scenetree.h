#ifndef SCENETREE_H
#define SCENETREE_H

#include <gtk/gtk.h>
#include <dengine/dengine.h>

void dengitor_scenetree_traverse(Scene* scene, GtkTreeStore* store);

void dengitor_scenetree_setup(GtkTreeView* tree);

#endif // SCENETREE_H
