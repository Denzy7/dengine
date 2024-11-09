#ifndef DENGITOR_VIEWPORTOPTS
#define DENGITOR_VIEWPORTOPTS

#include <gtk/gtk.h>

typedef struct
{
    GtkDialog* viewport_opts_dialog;
    float viewport_padding;
}DengitorViewportOpts;

void dengitor_viewportopts_setup(GtkBuilder* builder, DengitorViewportOpts* viewportopts);

#endif
