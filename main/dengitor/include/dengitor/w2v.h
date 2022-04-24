#ifndef W2V_H
#define W2V_H

/*
 * W2V provides a signal function
 * to convert data in a widget
 * to a variable
 *
 * also provides a GtkGLArea to
 * redraw once data is parsed to
 * variable
 */

#include <gtk/gtk.h>

void dengitor_w2v_set_glarea(GtkGLArea* area);

void dengitor_w2v_entry2float(GtkEntry* entry, float* var);

#endif // W2V_H
