#ifndef DENGITOR_GLAREA
#define DENGITOR_GLAREA

#include <gtk/gtk.h>

typedef enum
{
    DENGITOR_GLAREA_MODE_SCENE,
    DENGITOR_GLAREA_MODE_GAME
}GLAreaMode;

typedef struct 
{
    GtkGLArea* glarea;
    GLAreaMode glarea_mode;
    GtkEventBox* glarea_evbox;
    int glarea_evbox_first;
    //int glarea_evbox_rot;
/*    double glarea_evbox_x;*/
    /*double glarea_evbox_y;*/
    /*double glarea_evbox_dx;*/
    /*double glarea_evbox_dy;*/
    GtkRadioButton* toggle_game;
    GtkRadioButton* toggle_scene;
}DengitorGLArea;

void dengitor_glarea_setup(GtkBuilder* builder, DengitorGLArea* glarea);
#endif

