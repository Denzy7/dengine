#include "dengitor/dengitor.h"

int logready = 0;
void dengitor_log_callback(const char* logbuf, const char* tripbuf)
{
    if(!logready)
        return;
    Dengitor* dengitor = dengitor_get();
    GtkListStore* store = GTK_LIST_STORE(gtk_tree_view_get_model(dengitor->log.log));
    GtkTreeIter iter;
    gtk_list_store_append(store,&iter);
    gtk_list_store_set(store, &iter,
                       0, logbuf,
                       -1);
}

void dengitor_log_setup(GtkBuilder* builder, DengitorLog* log)
{
    dengineutils_logging_addcallback(dengitor_log_callback);
    log->log = GTK_TREE_VIEW( gtk_builder_get_object(builder, "log") );
    GtkListStore* store = gtk_list_store_new(1, G_TYPE_STRING);
    gtk_tree_view_set_model(log->log, GTK_TREE_MODEL(store));
    g_object_unref(store);
    GtkCellRenderer* renderer;
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(log->log,
                                                -1, "Log",
                                                renderer, "text",
                                                0, NULL);
    logready = 1;
}

DENGINE_EXPORT void dengitor_log_clear()
{
    Dengitor* dengitor = dengitor_get();
    GtkListStore* store = GTK_LIST_STORE(gtk_tree_view_get_model(dengitor->log.log));
    gtk_list_store_clear(store);
}
