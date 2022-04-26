#include "dengitor/dengitor.h"
#include "dengitor/scenetree.h"
#include "dengitor/w2v.h"

static Dengitor dengitor;
static const int prtbf_sz = 1024;
static char prtbf[1024];

int main(int argc, char *argv[])
{
    memset(&dengitor, 0, sizeof(Dengitor));
    dengitor.scene_grid_scale = 10.0f;
    dengitor.scene_grid_width = 0.5;
    dengitor.scene_grid_color[0] = 1.0f;
    dengitor.scene_grid_color[1] = 1.0f;
    dengitor.scene_grid_color[2] = 0.0f;
    dengitor.scene_grid_draw = 1;

    dengitor.scene_axis_scale = 2.0f;
    dengitor.scene_axis_width = 3.5f;
    dengitor.scene_entity_current_axis_width = 1.5;

    GtkApplication* app = gtk_application_new(
                "com.denzygames.Dengitor", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(dengitor_onactivate), NULL);
    int run = g_application_run( G_APPLICATION(app), argc, argv);
    return run;
}

gboolean dengitor_main_ontick(GtkWidget* widget, GdkFrameClock* clock, gpointer data)
{
    // this runs every frame cycle for the main GtkApplicationWindow
    // you can think of it as a "infinite while loop" until app closes
    gtk_widget_queue_draw( widget );
    return TRUE;
}

void dengitor_onactivate(GtkApplication* app)
{
    // MAIN UI
    dengitor.builder = gtk_builder_new_from_resource("/com/denzygames/Dengitor/dengine-editor-ui.glade");
    gtk_builder_connect_signals(dengitor.builder, NULL);

    dengitor.main = GTK_APPLICATION_WINDOW(gtk_builder_get_object(dengitor.builder, "main"));
    gtk_widget_add_tick_callback( GTK_WIDGET(dengitor.main), dengitor_main_ontick, NULL, NULL);

    // a blank cursor for hiding when dragging glarea
    // TODO : will this cause issues with multi-displays?
    dengitor.cursor_blank = gdk_cursor_new_for_display( gdk_display_get_default(), GDK_BLANK_CURSOR);
    dengitor.cursor_arrow = gdk_cursor_new_for_display( gdk_display_get_default(), GDK_ARROW);

    //  ABOUT WINDOW
    dengitor.about = GTK_ABOUT_DIALOG(gtk_builder_get_object(dengitor.builder, "about"));
    g_signal_connect(dengitor.about, "response", G_CALLBACK(dengitor_aboutdialog_hide), NULL);
    // menu button
    g_signal_connect(gtk_builder_get_object(dengitor.builder, "menu_action"),
                     "activate", G_CALLBACK(dengitor_aboutdialog_show), NULL);

    // GL AREA, ALL THE MAGIC IS HERE!
    dengitor.glarea = GTK_GL_AREA(gtk_builder_get_object(dengitor.builder, "glarea"));
    dengitor.glarea_evbox = GTK_EVENT_BOX( gtk_widget_get_parent( GTK_WIDGET(dengitor.glarea) ) );
    g_signal_connect(dengitor.glarea,
                     "realize", G_CALLBACK(dengitor_glarea_onrealize), NULL);
    g_signal_connect(dengitor.glarea,
                     "unrealize", G_CALLBACK(dengitor_glarea_onunrealize), NULL);
    g_signal_connect(dengitor.glarea,
                     "render", G_CALLBACK(dengitor_glarea_onrender), NULL);

    //event box for glarea
    g_signal_connect(dengitor.glarea_evbox, "motion-notify-event",
                     G_CALLBACK(dengitor_glarea_evbox_onmotion), NULL);
    g_signal_connect(dengitor.glarea_evbox, "button-press-event",
                     G_CALLBACK(dengitor_glarea_evbox_onbtnpress), NULL);
    g_signal_connect(dengitor.glarea_evbox, "button-release-event",
                     G_CALLBACK(dengitor_glarea_evbox_onbtnrelease), NULL);
    g_signal_connect(dengitor.main, "key-press-event",
                     G_CALLBACK(dengitor_glarea_evbox_onkeypress), NULL);

    dengitor.glarea_mode = DENGITOR_GLAREA_MODE_SCENE;
    dengitor.toggle_scene = GTK_TOGGLE_BUTTON( gtk_builder_get_object(dengitor.builder, "toggle_scene") );
    g_signal_connect(dengitor.toggle_scene, "toggled",
                     G_CALLBACK( dengitor_toggle_scenegame_ontoggle ), dengitor.toggle_scene);
    dengitor.toggle_game = GTK_TOGGLE_BUTTON( gtk_builder_get_object(dengitor.builder, "toggle_game") );
    g_signal_connect(dengitor.toggle_game, "toggled",
                     G_CALLBACK( dengitor_toggle_scenegame_ontoggle ), NULL);

    //  TREE VIEW TO SHOW SCENE ENTITIES
    dengitor.scene_treeview = GTK_TREE_VIEW(gtk_builder_get_object(dengitor.builder, "scene_treeview"));
    dengitor_scenetree_setup(dengitor.scene_treeview);
    dengitor.scene_treeview_store = GTK_TREE_STORE(gtk_tree_view_get_model(dengitor.scene_treeview));
    g_signal_connect(dengitor.scene_treeview,
                     "cursor-changed", G_CALLBACK(dengitor_scene_treeview_oncursorchange), NULL);

    //  PREFS WINDOW
    dengitor.prefs.builder = gtk_builder_new_from_resource("/com/denzygames/Dengitor/dengine-editor-prefs.glade");
    gtk_builder_connect_signals(dengitor.prefs.builder, NULL);
    dengitor_prefs_setup(dengitor.prefs.builder, &dengitor.prefs);
    // menu button prefs
    g_signal_connect(gtk_builder_get_object(dengitor.builder, "menu_prefs"),
                     "activate", G_CALLBACK(dengitor_prefs_show), &dengitor.prefs); 

    // setup inspector
    dengitor_inspector_setup(dengitor.builder, &dengitor.inspector);

    // setup viewportopts
    dengitor_viewport_opts_setup(dengitor.builder);

    // app setup complete..., show window and apply settings

    gtk_application_add_window(app, GTK_WINDOW(dengitor.main));
    gtk_widget_show_all(GTK_WIDGET(dengitor.main));

    // hide inspector
    dengitor_inspector_do_entity(NULL, &dengitor.inspector);

    GtkSettings* settings = gtk_settings_get_default();
    g_object_set(settings, "gtk-xft-antialias", 1, NULL);
    g_object_set(settings, "gtk-xft-hinting", 1, NULL);
    g_object_set(settings, "gtk-xft-hintstyle", "hintslight", NULL);
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

void dengitor_glarea_onrealize(GtkGLArea* area)
{
    GdkGLContext* context = gtk_gl_area_get_context(area);
    gdk_gl_context_make_current(context);
    if(gtk_gl_area_get_error(area) != NULL)
    {
        dengineutils_logging_log("ERROR::glarea error");
        return;
    }else
    {
        int maj, min;
        gdk_gl_context_get_version( context, &maj, &min);
        dengineutils_logging_log("INFO::GtkGLArea %d.%d realized", maj, min);
    }

    DengineInitOpts* opts = dengine_init_get_opts();
    opts->gl_loaddefault = 1;
    opts->window_createnative = 0;

    // preload glad to set viewport
    gladLoadGL();
    dengitor.glarea_alloc = g_new(GtkAllocation, 1);
    gtk_widget_get_allocation(GTK_WIDGET(area), dengitor.glarea_alloc);
    glViewport(0, 0, dengitor.glarea_alloc->width, dengitor.glarea_alloc->height);

    dengine_init();

    dengitor.scene_camera = denginescene_ecs_new_entity();
    Camera camera;
    dengine_camera_setup(&camera);
    dengine_camera_set_rendermode(DENGINE_CAMERA_RENDER_FOWARD, &camera);
    camera.clearcolor[0] = 0.5f;
    camera.clearonuse = 0;

    CameraComponent* camera_component = denginescene_ecs_new_cameracomponent(&camera);
    camera_component->last_cam = 0;
    dengitor.scene_camera->camera_component = camera_component;
    dengitor.scene_camera->transform.position[0] = 7.0f;
    dengitor.scene_camera->transform.position[1] = 7.0f;
    dengitor.scene_camera->transform.position[2] = 7.0f;
    dengitor.scene_camera->transform.rotation[0] = -45.0;
    dengitor.scene_camera->transform.rotation[1] = -135.0;
    g_signal_connect(dengitor.viewport_opts_fov,
                     "value-changed",
                     G_CALLBACK(dengitor_w2v_adjustment2float),
                     &camera_component->camera->fov);

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
    dengitor.scene_current = denginescene_new();

    // A SIMPLE SCENE
    Primitive cube;
    dengine_primitive_gen_cube(&cube, dengitor.shader_standard);

    Material std_mat;
    Material sky_equireq;

    dengine_material_setup(&sky_equireq);
    dengine_material_set_shader_color(dengitor.shader_skybox_2d, &sky_equireq);

    // TODO : memory leak if not destroyed!
    dengine_material_setup(&std_mat);
    dengine_material_set_shader_color(dengitor.shader_standard, &std_mat);
    dengine_material_set_shader_shadow(dengitor.shader_shadow2d, &std_mat);

    //skybox
    Texture equireq;
    memset(&equireq, 0, sizeof(Texture));

    snprintf(prtbf, prtbf_sz,
             "%s/textures/hdri/sunset.hdr",
             dengineutils_filesys_get_assetsdir());

    uint32_t type = GL_FLOAT;
    equireq.interface = DENGINE_TEXTURE_INTERFACE_FLOAT;
    if(!dengine_texture_issupprorted(GL_TEXTURE_2D, GL_FLOAT, GL_RGB, GL_RGB))
    {
        dengineutils_logging_log("WARNING::GPU has no float texture support! Falling back to 8-bit");
        equireq.interface = DENGINE_TEXTURE_INTERFACE_8_BIT;
        type = GL_UNSIGNED_BYTE;
    }

    dengine_texture_load_file(prtbf, 1, &equireq);

    equireq.type = type;
    equireq.filter_min = GL_LINEAR;
    equireq.format = GL_RGB;
    equireq.internal_format = GL_RGB;
    dengine_texture_gen(1, &equireq);
    dengine_texture_bind(GL_TEXTURE_2D, &equireq);
    dengine_texture_data(GL_TEXTURE_2D, &equireq);
    dengine_texture_set_params(GL_TEXTURE_2D, &equireq);
    dengine_texture_bind(GL_TEXTURE_2D, NULL);
    dengine_texture_free_data(&equireq);

    dengine_material_set_texture(&equireq, "eqireqMap", &sky_equireq);
    Skybox* skybox = denginescene_new_skybox(&cube, &sky_equireq);
    dengitor.scene_current->skybox = skybox;

    Entity* cube_ent = denginescene_ecs_new_entity();
    MeshComponent* cube_mesh = denginescene_ecs_new_meshcomponent(&cube, &std_mat);
    cube_ent->mesh_component = cube_mesh;

    denginescene_add_entity(dengitor.scene_current, cube_ent);

    Primitive plane;
    dengine_primitive_gen_plane(&plane, dengitor.shader_standard);

    Material plane_mat;
    dengine_material_setup(&plane_mat);
    dengine_material_set_shader_color(dengitor.shader_standard, &plane_mat);
    dengine_material_set_shader_shadow(dengitor.shader_shadow2d, &plane_mat);

    static const char* plane_tex_files_tgt[][2]=
    {
        {"brickwall.jpg", "diffuseTex"},
        {"brickwall_normal.jpg", "normalTex"}
    };

    Texture plane_tex[2];
    memset(plane_tex, 0, sizeof(plane_tex));

    for(int i = 0; i < DENGINE_ARY_SZ(plane_tex_files_tgt); i++)
    {
        Texture* tex = &plane_tex[i];
        tex->auto_dataonload=1;
        tex->interface=DENGINE_TEXTURE_INTERFACE_8_BIT;
        snprintf(prtbf,prtbf_sz,"%s/textures/2d/%s",dengineutils_filesys_get_assetsdir(), plane_tex_files_tgt[i][0]);
        dengine_texture_load_file(prtbf,1, tex);
        dengine_material_set_texture(tex, plane_tex_files_tgt[i][1], &plane_mat);
    }

    Entity* plane_ent = denginescene_ecs_new_entity();
    MeshComponent* plane_mesh = denginescene_ecs_new_meshcomponent(&plane, &plane_mat);
    plane_ent->mesh_component = plane_mesh;

    plane_ent->transform.position[1] = -1.0f;

    plane_ent->transform.scale[0] = 5.0f;
    plane_ent->transform.scale[1] = 5.0f;
    plane_ent->transform.scale[2] = 5.0f;

    Entity* ch = denginescene_ecs_new_entity();
    MeshComponent* ch_mesh = denginescene_ecs_new_meshcomponent(&cube, &std_mat);
    ch->mesh_component = ch_mesh;
    ch->transform.scale[0] = 0.5f;
    ch->transform.scale[1] = 0.5f;
    ch->transform.scale[2] = 0.5f;

    ch->transform.position[1] = 2.5f;

    denginescene_ecs_set_entity_name(ch, "child entity");
    denginescene_ecs_set_entity_name(cube_ent, "parent entity");

    char nm[512];
    for(int i = 0; i < 2; i++)
    {
        Entity* ch_l1 = denginescene_ecs_new_entity();
        g_snprintf(nm, sizeof(nm), "child lv %d", i);
        denginescene_ecs_set_entity_name(ch_l1, nm);
        denginescene_ecs_parent(ch, ch_l1);
        for(int j = 0; j < 3; j++)
        {
            Entity* ch_l2 = denginescene_ecs_new_entity();
            g_snprintf(nm, sizeof(nm), "child lv %d:%d", i, j);
            denginescene_ecs_set_entity_name(ch_l2, nm);
            denginescene_ecs_parent(ch_l1, ch_l2);
            if(i == 1)
            {
                for(int k = 0; k < 4; k++)
                {
                    Entity* ch_l3 = denginescene_ecs_new_entity();
                    g_snprintf(nm, sizeof(nm), "child lv %d:%d:%d", i, j, k);
                    denginescene_ecs_set_entity_name(ch_l3, nm);
                    denginescene_ecs_parent(ch_l2, ch_l3);
                }
            }
        }
    }

    denginescene_ecs_parent(cube_ent, ch);

    denginescene_add_entity(dengitor.scene_current, plane_ent);

    Camera camera2;
    dengine_camera_setup(&camera2);
    dengine_camera_set_rendermode(DENGINE_CAMERA_RENDER_FOWARD, &camera2);

    CameraComponent* cam2_comp = denginescene_ecs_new_cameracomponent(&camera2);
    Entity* cam_ent = denginescene_ecs_new_entity();
    cam_ent->transform.position[0] = 7.0f;
    cam_ent->transform.position[1] = 5.0f;
    cam_ent->transform.rotation[0] = -45.0;
    cam_ent->transform.rotation[1] = 180.0;
    cam_ent->camera_component = cam2_comp;
    denginescene_ecs_set_entity_name(cam_ent, "this is a camera");
    denginescene_add_entity(dengitor.scene_current, cam_ent);

    DirLight dLight;
    memset(&dLight, 0, sizeof(DirLight));
    dLight.shadow.enable = 1;
    dLight.shadow.shadow_map_size = 512;
    dengine_lighting_light_setup(DENGINE_LIGHT_DIR, &dLight);
    dLight.light.strength = 1.2f;
    //sun? (https://www.htmlcsscolor.com/hex/EF8E38) : 239.0 / 255.0, 142.0 / 255.0, 56.0 / 255.0
    dLight.light.diffuse[0] = 239.0f / 255.0f;
    dLight.light.diffuse[1] =  142.0f / 255.0f;
    dLight.light.diffuse[2] = 56.0f / 255.0f;

    LightComponent* light_comp = denginescene_ecs_new_lightcomponent(DENGINE_LIGHT_DIR, &dLight);
    Entity* dLight_ent = denginescene_ecs_new_entity();
    denginescene_ecs_set_entity_name(dLight_ent, "this is a dir light");
    dLight_ent->transform.position[0] = 3.0f;
    dLight_ent->transform.position[1] = 3.0f;
    dLight_ent->transform.position[2] = -3.0f;
    dLight_ent->light_component = light_comp;
    denginescene_add_entity(dengitor.scene_current, dLight_ent);

    PointLight pLight;
    memset(&pLight, 0, sizeof(PointLight));
    dengine_lighting_light_setup(DENGINE_LIGHT_POINT, &pLight);
    pLight.light.diffuse[0] = 0;
    light_comp = denginescene_ecs_new_lightcomponent(DENGINE_LIGHT_POINT, &pLight);
    Entity* pLight_ent = denginescene_ecs_new_entity();
    denginescene_ecs_set_entity_name(pLight_ent, "this is a point light");
    pLight_ent->light_component = light_comp;
    pLight_ent->transform.position[0] = -3.0f;
    pLight_ent->transform.position[1] = 3.0f;
    pLight_ent->transform.position[2] = 3.0f;
    denginescene_add_entity(dengitor.scene_current, pLight_ent);

    dengitor_scenetree_traverse(dengitor.scene_current, dengitor.scene_treeview_store);
#endif

    snprintf(prtbf, prtbf_sz, "Dengitor - v%s --  GL : %s, RENDERER : %s",
             DENGINE_VERSION,
             (char*)glGetString(GL_VERSION),
             (char*)glGetString(GL_RENDERER));;
    gtk_window_set_title( GTK_WINDOW(dengitor.main), prtbf);
}

void dengitor_glarea_onunrealize(GtkGLArea* area)
{
    GdkGLContext* context = gtk_gl_area_get_context(area);
    gdk_gl_context_make_current(context);
    if(gtk_gl_area_get_error(area) != NULL)
    {
        dengineutils_logging_log("ERROR::glarea error unrealize");
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

    g_free(dengitor.glarea_alloc);
    g_object_unref(dengitor.cursor_blank);
    g_object_unref(dengitor.cursor_arrow);

    if(dengitor.scene_current)
        denginescene_destroy(dengitor.scene_current);

    dengine_terminate();
}

void dengitor_glarea_evbox_onmotion(GtkEventBox* evbox, GdkEventMotion* motion)
{
    if(dengitor.glarea_evbox_rot)
    {
        gtk_widget_get_allocation(GTK_WIDGET(dengitor.glarea), dengitor.glarea_alloc);
        //h for inverting gdk window coords
        double h = dengitor.glarea_alloc->height;
        if(!dengitor.glarea_evbox_first)
        {
            dengitor.glarea_evbox_x = motion->x;
            dengitor.glarea_evbox_y = h - motion->y;
            dengitor.glarea_evbox_first = 1;
        }

        dengitor.glarea_evbox_dx = motion->x - dengitor.glarea_evbox_x;
        dengitor.glarea_evbox_dy = (h - motion->y) - dengitor.glarea_evbox_y;
        dengitor.glarea_evbox_x = motion->x;
        dengitor.glarea_evbox_y = h - motion->y;

        dengitor.scene_camera->transform.rotation[0] += dengitor.glarea_evbox_dy;
        dengitor.scene_camera->transform.rotation[1] += dengitor.glarea_evbox_dx;

        float max_x = 89.9f;
        dengitor.scene_camera->transform.rotation[0] = glm_clamp(dengitor.scene_camera->transform.rotation[0],
                -max_x, max_x);
    }
}

void dengitor_glarea_evbox_onbtnpress(GtkEventBox* evbox, GdkEventButton* button)
{
    if(button->button == GDK_BUTTON_SECONDARY)
    {
        dengitor.glarea_evbox_rot = 1;

        GdkWindow* window = gtk_widget_get_window( GTK_WIDGET(dengitor.glarea) );
        gdk_window_set_cursor(window, dengitor.cursor_blank);
    }
}

void dengitor_glarea_evbox_onbtnrelease(GtkEventBox* evbox, GdkEventButton* button)
{
    dengitor.glarea_evbox_first = 0;
    dengitor.glarea_evbox_rot = 0;
    GdkWindow* window = gtk_widget_get_window( GTK_WIDGET(dengitor.glarea) );
    gdk_window_set_cursor(window, dengitor.cursor_arrow);
}

void dengitor_glarea_evbox_onkeypress(GtkEventBox* evbox, GdkEventKey* key)
{
    vec3 front;
    denginescene_ecs_get_front(dengitor.scene_camera, front);

    if(dengitor.glarea_evbox_rot && key->keyval == GDK_KEY_w || key->keyval == GDK_KEY_W)
    {
        for(int i = 0; i < 3; i++)
        {
            dengitor.scene_camera->transform.position[i] = front[i];
        }
    }
}

void dengitor_glarea_onrender(GtkGLArea* area)
{
    GdkGLContext* context = gtk_gl_area_get_context(area);
    gdk_gl_context_make_current(context);

    // SCENE CAMERA PASS
    int x, y, w, h;
    dengine_viewport_get(&x, &y, &w, &h);

    Camera* out = NULL;

    // process scene
    denginescene_update(dengitor.scene_current);

    Entity* current_ent = dengitor.scene_entity_current;

    if(dengitor.glarea_mode == DENGITOR_GLAREA_MODE_SCENE)
    {
        Framebuffer entry_fb;
        dengine_entrygl_framebuffer(GL_FRAMEBUFFER, &entry_fb);

        Camera* scene_camera = dengitor.scene_camera->camera_component->camera;
        out = scene_camera;
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
        vec3 front;
        denginescene_ecs_get_front(dengitor.scene_camera, front);
        dengine_camera_lookat(front,
                              scene_camera);
        dengine_camera_apply(dengitor.shader_default, scene_camera);

        static mat4 mat_4;
        static vec3 vec_3;
        float init_width;
        glGetFloatv(GL_LINE_WIDTH, &init_width);
        if(dengitor.scene_grid_draw)
        {
            glm_mat4_identity(mat_4);
            // DRAW grid
            vec_3[0] = dengitor.scene_grid_scale;
            vec_3[1] = dengitor.scene_grid_scale;
            vec_3[2] = dengitor.scene_grid_scale;

            glm_scale(mat_4, vec_3);
            dengine_shader_set_vec3(dengitor.shader_default, "color", dengitor.scene_grid_color);
            dengine_shader_set_mat4(dengitor.shader_default, "model",mat_4[0]);

            glLineWidth(dengitor.scene_grid_width);
            dengine_draw_primitive(&dengitor.scene_grid, dengitor.shader_default);
        }

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

        if(dengitor.scene_current)
        {
            denginescene_ecs_do_camera_scene(dengitor.scene_camera, dengitor.scene_current);
        }

        // draw scene origin axis
        int dfunc;
        glGetIntegerv(GL_DEPTH_FUNC, &dfunc);

        // these axes will always be drawn on top of everything
        glDepthFunc(GL_ALWAYS);

        glLineWidth(dengitor.scene_axis_width);
        dengitor_draw_axis(&dengitor.scene_axis, dengitor.shader_default);

        //draw a local axis for current entity
        if(current_ent)
        {
            glLineWidth(dengitor.scene_entity_current_axis_width);
            dengine_shader_set_mat4(dengitor.shader_default, "model", current_ent->transform.world_model[0]);
            dengitor_draw_axis(&dengitor.scene_axis, dengitor.shader_default);
        }
        glDepthFunc(dfunc);

        glLineWidth(init_width);

        dengine_framebuffer_bind(GL_FRAMEBUFFER, &entry_fb);
        dengine_viewport_set(x, y, w, h);
    }else
    {
        if(dengitor.scene_current)
        {
            out = dengitor.scene_current->last_cam;
        }
    }

    // SCREEN QUAD PASS
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static vec4 viewport_color = {0.0f, 0.0f, 0.0f, 1.0f};
    static vec4 red = {1.0f, 0.0f, 0.0f, 1.0f};
    if(out)
        denginegui_panel(0, 0, w, h, out->framebuffer.color, NULL, viewport_color);
    else
        denginegui_text(10, (float)h / 2.0f, "NO CAMERA FOUND!", red);

    denginegui_text(10, 10, (const char*)glGetString(GL_VERSION) , NULL);
}

void dengitor_toggle_scenegame_ontoggle(GtkToggleButton* toggle_btn, gpointer flag)
{
    if(gtk_toggle_button_get_active(toggle_btn))
    {
        if(flag)
        {
            gtk_toggle_button_set_active(dengitor.toggle_game, false);
            dengitor.glarea_mode = DENGITOR_GLAREA_MODE_SCENE;
        }else
        {
            gtk_toggle_button_set_active(dengitor.toggle_scene, false);
            dengitor.glarea_mode = DENGITOR_GLAREA_MODE_GAME;
        }
    }
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

void dengitor_scene_treeview_oncursorchange(GtkTreeView* tree)
{
    GtkTreeModel* model = gtk_tree_view_get_model(tree);
    GtkTreeSelection* selection = gtk_tree_view_get_selection(tree);
    GtkTreeIter iter;
    if(gtk_tree_selection_get_selected(selection, &model,&iter))
    {
        char* name;
        gtk_tree_model_get(model, &iter,
                           DENGITOR_SCENETREE_ENTNAME, &name, -1);
        Entity* current = NULL;
        gtk_tree_model_get(model, &iter,
                           DENGITOR_SCENETREE_ENTPTR, &current, -1);
        dengineutils_logging_log("selected %s %p %u", name, current, current->entity_id);

        dengitor_inspector_do_entity(current, &dengitor.inspector);

        dengitor.scene_entity_current = current;

        free(name);
    }
}

void dengitor_viewport_opts_setup(GtkBuilder* builder)
{
    dengitor.viewport_opts = GTK_DIALOG( gtk_builder_get_object(builder, "viewport_opts") );

    dengitor.viewport_opts_fov = GTK_ADJUSTMENT( gtk_builder_get_object(builder, "viewport_opts_fov_adjustment") );
    dengitor.viewport_opts_grid_colour = GTK_COLOR_BUTTON( gtk_builder_get_object(builder, "viewport_opts_grid_colour") );
    g_signal_connect(dengitor.viewport_opts_grid_colour,
                     "color-set",
                     G_CALLBACK(dengitor_w2v_colorbtn2float4),
                     dengitor.scene_grid_color);
    dengitor.viewport_opts_grid_scale = GTK_ADJUSTMENT( gtk_builder_get_object(builder, "viewport_opts_grid_scale") );
    g_signal_connect(dengitor.viewport_opts_grid_scale,
                     "value-changed",
                     G_CALLBACK(dengitor_w2v_adjustment2float),
                     &dengitor.scene_grid_scale);
    dengitor.viewport_opts_grid_width = GTK_ADJUSTMENT( gtk_builder_get_object(builder, "viewport_opts_grid_width") );
    g_signal_connect(dengitor.viewport_opts_grid_width,
                     "value-changed",
                     G_CALLBACK(dengitor_w2v_adjustment2float),
                     &dengitor.scene_grid_width);
    dengitor.viewport_opts_grid_draw = GTK_TOGGLE_BUTTON( gtk_builder_get_object(builder, "viewport_opts_grid_draw") );
    g_signal_connect(
                dengitor.viewport_opts_grid_draw,
                "toggled",
                G_CALLBACK(dengitor_viewport_opts_grid_draw_ontoggle),
                NULL);

    g_signal_connect_swapped(
                gtk_builder_get_object(builder, "viewport_opts_btn"),
                "clicked",
                G_CALLBACK(dengitor_viewport_opts_show),
                dengitor.viewport_opts
                );

    g_signal_connect_swapped(
                gtk_builder_get_object(builder, "viewport_opts_ok"),
                "clicked",
                G_CALLBACK(gtk_widget_hide),
                dengitor.viewport_opts
                );
}

void dengitor_viewport_opts_show(GtkDialog* viewportopts)
{
    gtk_widget_show_all( GTK_WIDGET(viewportopts) );
    Camera* scene_cam = dengitor.scene_camera->camera_component->camera;

    //set opts
    gtk_toggle_button_set_active(dengitor.viewport_opts_grid_draw, dengitor.scene_grid_draw);
    gtk_adjustment_set_value(dengitor.viewport_opts_fov, scene_cam->fov);
    gtk_adjustment_set_value(dengitor.viewport_opts_grid_width, dengitor.scene_grid_width);
    gtk_adjustment_set_value(dengitor.viewport_opts_grid_scale, dengitor.scene_grid_scale);
}

void dengitor_viewport_opts_grid_draw_ontoggle(GtkToggleButton* toggle_btn)
{
    dengitor.scene_grid_draw = gtk_toggle_button_get_active( toggle_btn);
}
