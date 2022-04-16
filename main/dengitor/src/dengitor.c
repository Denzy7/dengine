#include "dengitor/dengitor.h"

static Dengitor dengitor;

int main(int argc, char *argv[])
{
    memset(&dengitor, 0, sizeof(Dengitor));
    dengitor.scene_grid_scale = 10.0f;
    dengitor.scene_grid_width = 2.0f;
    dengitor.scene_grid_color[0] = 1.0f;
    dengitor.scene_grid_color[1] = 1.0f;
    dengitor.scene_grid_color[2] = 0.0f;

    dengitor.scene_axis_scale = 2.0f;
    dengitor.scene_axis_width = 3.5f;

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
    camera.clearcolor[0] = 0.5f;
    camera.clearonuse = 0;

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

    dengine_primitive_gen_grid(10, &dengitor.scene_grid, dengitor.shader_default);
    dengine_primitive_gen_axis(&dengitor.scene_axis, dengitor.shader_default);
    dengitor.scene_axis.index_count = 2;

#if 1
    // A SIMPLE SCENE
    Primitive cube;
    dengine_primitive_gen_cube(&cube, dengitor.shader_debug_normals);

    Material dbg_norm_mat;

    // TODO : memory leak if not destroyed!
    dengine_material_setup(&dbg_norm_mat);
    dengine_material_set_shader_color(dengitor.shader_debug_normals, &dbg_norm_mat);

    Entity* cube_ent = denginescene_ecs_new_entity();
    MeshComponent* cube_mesh = denginescene_ecs_new_meshcomponent(&cube, &dbg_norm_mat);
    cube_ent->mesh_component = cube_mesh;

    dengitor.scene_current = denginescene_new();
    denginescene_add_entity(dengitor.scene_current, cube_ent);

    Primitive plane;
    dengine_primitive_gen_plane(&plane, dengitor.shader_debug_normals);

    Entity* plane_ent = denginescene_ecs_new_entity();
    MeshComponent* plane_mesh = denginescene_ecs_new_meshcomponent(&plane, &dbg_norm_mat);
    plane_ent->mesh_component = plane_mesh;

    plane_ent->transform.position[1] = -1.0f;

    plane_ent->transform.scale[0] = 5.0f;
    plane_ent->transform.scale[1] = 5.0f;
    plane_ent->transform.scale[2] = 5.0f;

    denginescene_add_entity(dengitor.scene_current, plane_ent);
#endif
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

    if(dengitor.scene_current)
        denginescene_destroy(dengitor.scene_current);

    dengine_terminate();
}

void dengitor_scene_glarea_onrender(GtkGLArea* area)
{
    // SCENE CAMERA PASS
    int x, y, w, h;
    dengine_viewport_get(&x, &y, &w, &h);

    int last_fb;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &last_fb);

    Camera* scene_camera = dengitor.scene_camera->camera_component->camera;
    dengine_camera_use(scene_camera);
    float r,g,b,a;
    dengine_framebuffer_get_clearcolor(&r, &g, &b, &a);
    glClearColor(
            scene_camera->clearcolor[0],
            scene_camera->clearcolor[1],
            scene_camera->clearcolor[2],
            scene_camera->clearcolor[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(r, g, b, a);
    dengine_viewport_set(0, 0,
                         scene_camera->render_width, scene_camera->render_height);

    if(dengitor.scene_camera_last_w != w || dengitor.scene_camera_last_h != h)
    {
        dengine_camera_resize(scene_camera, w, h);
        dengitor.scene_camera_last_w = w;
        dengitor.scene_camera_last_h = h;
        dengineutils_logging_log("INFO::resize scene camera to %dx%d", w, h);dengitor.scene_camera_last_w = w;
    }

    dengine_camera_project_perspective( (float)w / (float)h,
                                        scene_camera);
    dengine_camera_lookat(NULL,
                          scene_camera);
    dengine_camera_apply(dengitor.shader_default, scene_camera);

    static mat4 mat_4;
    glm_mat4_identity(mat_4);

    // DRAW grid
    vec3 vec_3 = {dengitor.scene_grid_scale,
                             dengitor.scene_grid_scale,
                             dengitor.scene_grid_scale};
    glm_scale(mat_4, vec_3);
    dengine_shader_set_vec3(dengitor.shader_default, "color", dengitor.scene_grid_color);
    dengine_shader_set_mat4(dengitor.shader_default, "model",mat_4[0]);
    float init_width;
    glGetFloatv(GL_LINE_WIDTH, &init_width);
    glLineWidth(dengitor.scene_grid_width);
    dengine_draw_primitive(&dengitor.scene_grid, dengitor.shader_default);

    glm_mat4_identity(mat_4);

    vec_3[0] = 0.0f;
    vec_3[1] = 0.01f;
    vec_3[2] = 0.0f;
    glm_translate(mat_4, vec_3);

    vec_3[0] = dengitor.scene_axis_scale;
    vec_3[1] = dengitor.scene_axis_scale;
    vec_3[2] = dengitor.scene_axis_scale;
    glm_scale(mat_4, vec_3);
    dengine_shader_set_mat4(dengitor.shader_default, "model",mat_4[0]);

    glLineWidth(dengitor.scene_axis_width);
    dengitor_draw_axis(&dengitor.scene_axis, dengitor.shader_default);

    glLineWidth(init_width);

    glBindFramebuffer(GL_FRAMEBUFFER, last_fb);
    dengine_viewport_set(x, y, w, h);

    if(dengitor.scene_current)
    {
        denginescene_ecs_do_camera_scene(dengitor.scene_camera, dengitor.scene_current);
        denginescene_update(dengitor.scene_current);
    }

    // SCREEN QUAD PASS
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static vec4 viewport_color = {0.0f, 0.0f, 0.0f, 1.0f};
    denginegui_panel(0, 0, w, h, scene_camera->framebuffer.color, NULL, viewport_color);

    denginegui_text(10, 10, (const char*)glGetString(GL_VERSION) , NULL);
}

void dengitor_draw_axis(Primitive* axis, Shader* shader)
{
    float color[3];
    for (int i = 0; i < 3; i++) {
        color[0] = i == 0 ? 1.0f : 0.0f, color[1] = i == 1 ? 1.0f : 0.0f, color[2] = i == 2 ? 1.0f : 0.0f;
        axis->offset = (void*)(i*2*sizeof (uint16_t));
        dengine_shader_set_vec3(shader, "color", color);
        dengine_draw_primitive(axis, shader);
    }
}
