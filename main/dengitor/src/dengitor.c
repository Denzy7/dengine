#include "dengitor/dengitor.h"

static Dengitor dengitor;

int main(int argc, char *argv[])
{

    GtkApplication* app = gtk_application_new(
                "com.denzygames.Dengitor", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(dengitor_onactivate), NULL);
    int run = g_application_run( G_APPLICATION(app), argc, argv);
    return run;
}

void dengitor_onactivate(GtkApplication* app)
{
    //TODO : compile resources
    const char* ui = "/home/denzy/dengine/main/dengitor/res/default/dengine-editor-ui.glade";

    dengitor.builder = gtk_builder_new_from_file(ui);

    dengitor.main = GTK_APPLICATION_WINDOW(gtk_builder_get_object(dengitor.builder, "main"));

    dengitor.about = GTK_ABOUT_DIALOG(gtk_builder_get_object(dengitor.builder, "about"));
    g_signal_connect(dengitor.about, "response", G_CALLBACK(dengitor_aboutdialog_hide), NULL);
    // menu button
    g_signal_connect(gtk_builder_get_object(dengitor.builder, "menu_action"),
                     "activate", G_CALLBACK(dengitor_aboutdialog_show), NULL);

    dengitor.scene_glarea = GTK_GL_AREA(gtk_builder_get_object(dengitor.builder, "scene_glarea"));
    g_signal_connect(dengitor.scene_glarea,
                     "realize", G_CALLBACK(dengitor_scene_glarea_onrealize), NULL);
    g_signal_connect(dengitor.scene_glarea,
                     "unrealize", G_CALLBACK(dengitor_scene_glarea_onunrealize), NULL);
    g_signal_connect(dengitor.scene_glarea,
                     "render", G_CALLBACK(dengitor_scene_glarea_onrender), NULL);

    gtk_application_add_window(app, GTK_WINDOW(dengitor.main));
    gtk_widget_show_all(GTK_WIDGET(dengitor.main));
}

void dengitor_aboutdialog_show()
{
    gtk_about_dialog_set_version(dengitor.about, DENGINE_VERSION);
    gtk_widget_show_all(GTK_WIDGET(dengitor.about));
}

void dengitor_aboutdialog_hide()
{
    gtk_widget_hide(GTK_WIDGET(dengitor.about));
}

void dengitor_scene_glarea_onrealize(GtkGLArea* area)
{
    gtk_gl_area_make_current(area);
    if(gtk_gl_area_get_error(area) != NULL)
    {
        dengineutils_logging_log("ERROR::Cannot make_current glarea");
        return;
    }else
    {
        int maj, min;
        gdk_gl_context_get_version( gtk_gl_area_get_context(area), &maj, &min);
        dengineutils_logging_log("INFO::GtkGLArea %d.%d realized", maj, min);
    }

    DengineInitOpts* opts = dengine_init_get_opts();
    opts->gl_loaddefault = 1;
    opts->window_createnative = 0;

    // preload glad to set viewport
    gladLoadGL();
    GtkAllocation* alloc = g_new(GtkAllocation, 1);
    gtk_widget_get_allocation(GTK_WIDGET(area), alloc);
    glViewport(0, 0, alloc->width, alloc->height);
    g_free(alloc);

    dengine_init();

    dengitor.scene_camera = denginescene_ecs_new_entity();
    Camera camera;
    dengine_camera_setup(&camera);
    dengine_camera_set_rendermode(DENGINE_CAMERA_RENDER_FOWARD, &camera);

    CameraComponent* camera_component = denginescene_ecs_new_cameracomponent(&camera);
    dengitor.scene_camera->camera_component = camera_component;
    dengitor.scene_camera->transform.position[0] = 7.0f;
    dengitor.scene_camera->transform.position[1] = 7.0f;
    dengitor.scene_camera->transform.position[2] = 7.0f;

    // compile some standard shaders
    dengitor.shader_default = dengine_shader_new_shader_standard(DENGINE_SHADER_DEFAULT);
    dengitor.shader_standard = dengine_shader_new_shader_standard(DENGINE_SHADER_STANDARD);
    dengitor.shader_shadow2d = dengine_shader_new_shader_standard(DENGINE_SHADER_SHADOW2D);
    dengitor.shader_shadow3d = dengine_shader_new_shader_standard(DENGINE_SHADER_SHADOW3D);
    dengitor.shader_debug_normals = dengine_shader_new_shader_standard(DENGINE_SHADER_DEBUG_NORMALS);
    dengitor.shader_skybox_cube = dengine_shader_new_shader_standard(DENGINE_SHADER_SKYBOXCUBE);
    dengitor.shader_skybox_2d = dengine_shader_new_shader_standard(DENGINE_SHADER_SKYBOX2D);
}

void dengitor_scene_glarea_onunrealize(GtkGLArea* area)
{
    gtk_gl_area_make_current(area);
    if(gtk_gl_area_get_error(area) != NULL)
    {
        dengineutils_logging_log("ERROR::Cannot make_current glarea");
        return;
    }else
    {
        dengineutils_logging_log("INFO::GtkGLArea unrealized");
    }

    // clean scene cam
    denginescene_ecs_destroy_entity(dengitor.scene_camera);

    free(dengitor.shader_default);
    free(dengitor.shader_standard);
    free(dengitor.shader_shadow2d);
    free(dengitor.shader_shadow3d);
    free(dengitor.shader_debug_normals);
    free(dengitor.shader_skybox_cube);
    free(dengitor.shader_skybox_2d);

    dengine_terminate();
}

void dengitor_scene_glarea_onrender(GtkGLArea* area)
{
    glClearColor(0.0, 0.5, 0.3, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int x, y, w, h;
    dengine_viewport_get(&x, &y, &w, &h);

    dengine_camera_project_perspective( (float)w / (float)h,
                                        dengitor.scene_camera->camera_component->camera);
    dengine_camera_lookat(NULL,
                          dengitor.scene_camera->camera_component->camera);
    static float rgba[4] = {1.0, 1.0, 0.0, 1.0};
    denginegui_text(10, 10, (const char*)glGetString(GL_VERSION) , rgba);
}
