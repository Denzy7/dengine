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

void dengitor_w2v_colorbtn2float4(GtkColorButton* colbtn, float* var)
{
    GdkRGBA col;
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(colbtn), &col);

    var[0] = col.red;
    var[1] = col.green;
    var[2] = col.blue;
    var[3] = col.alpha;
}
