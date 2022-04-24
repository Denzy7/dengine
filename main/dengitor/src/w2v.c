#include "dengitor/w2v.h"

static GtkGLArea* _area = NULL;

void dengitor_w2v_set_glarea(GtkGLArea* area)
{
    _area = area;
}

void dengitor_w2v_entry2float(GtkEntry* entry, float* var)
{
    const char* text;
    GtkEntryBuffer* buffer;
    buffer = gtk_entry_get_buffer(entry);
    text = gtk_entry_buffer_get_text(buffer);
    *var = strtof(text, NULL);

    if(_area)
        gtk_widget_queue_draw( GTK_WIDGET(_area) );

}
