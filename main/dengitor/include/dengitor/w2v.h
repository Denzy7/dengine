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

void dengitor_w2v_adjustment2int(GtkAdjustment* adjustment, int* var);

void dengitor_w2v_colorbtn2float4(GtkColorButton* colbtn, float* var);

void dengitor_w2v_combo2int(GtkComboBox* combo, int* var);

void dengitor_w2v_toggle2int(GtkToggleButton* toggle, int* var);

void dengitor_w2v_check2int(GtkCheckButton* check, int* var);

#endif // W2V_H
