#ifndef DENGITOR_LOG
#define DENGITOR_LOG

#include <gtk/gtk.h>
typedef struct 
{
    GtkTreeView* log;
}DengitorLog;

void dengitor_log_setup(GtkBuilder* builder, DengitorLog* log);

#endif
