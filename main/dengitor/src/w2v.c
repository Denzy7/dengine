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
