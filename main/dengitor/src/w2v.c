#include "dengitor/w2v.h"

void dengitor_w2v_entry2float(GtkEntry* entry, float* var)
{
    const char* text;
    GtkEntryBuffer* buffer;
    buffer = gtk_entry_get_buffer(entry);
    text = gtk_entry_buffer_get_text(buffer);
    sscanf(text, "%f", var);
}

void dengitor_w2v_adjustment2float(GtkAdjustment* adjustment, float* var)
{
    *var = gtk_adjustment_get_value(adjustment);
}

void dengitor_w2v_adjustment2int(GtkAdjustment* adjustment, int* var)
{
    float v = gtk_adjustment_get_value(adjustment);
    *var = (int)v;
}

void dengitor_w2v_colorbtn2float4(GtkColorButton* colbtn, float* var)
{
    GdkRGBA col;
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(colbtn), &col);

    var[0] = col.red;
    var[1] = col.green;
    var[2] = col.blue;
    var[3] = col.alpha;
}

void dengitor_w2v_combo2int(GtkComboBox* combo, int* var)
{
    *var = gtk_combo_box_get_active(combo);
}

void dengitor_w2v_toggle2int(GtkToggleButton* toggle, int* var)
{
    *var = gtk_toggle_button_get_active(toggle);
}

void dengitor_w2v_check2int(GtkCheckButton* check, int* var)
{
    dengitor_w2v_toggle2int( GTK_TOGGLE_BUTTON(check), var);
}
