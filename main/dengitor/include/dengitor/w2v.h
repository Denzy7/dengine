#ifndef W2V_H
#define W2V_H

/*
 * W2V provides a signal function
 * to convert data in a widget
 * to a variable
 */

#include <gtk/gtk.h>

void dengitor_w2v_entry2float(GtkEntry* entry, float* var);

void dengitor_w2v_adjustment2float(GtkAdjustment* adjustment, float* var);

void dengitor_w2v_colorbtn2float4(GtkColorButton* colbtn, float* var);

#endif // W2V_H
