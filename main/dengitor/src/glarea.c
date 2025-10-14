#include "dengitor/dengitor.h"

DENGINE_EXPORT void dengitor_glearea_onevent()
{
    Dengitor* dengitor = dengitor_get();
    Entity* scenecam = dengitor->scene_camera;
    if(!scenecam)
        return;

    for(size_t i = 0; i < scenecam->scripts.count; i++)
    {
        denginescene_ecs_scripts_run(scenecam, DENGINE_SCRIPT_FUNC_UPDATE, scenecam);
    }
    dengitor_redraw();
}
void dengitor_glarea_draw_axis(Primitive* axis, Shader* shader)
{
    float color[3];
    for (int i = 0; i < 3; i++) {
        color[0] = i == 0 ? 1.0f : 0.0f, color[1] = i == 1 ? 1.0f : 0.0f, color[2] = i == 2 ? 1.0f : 0.0f;
        axis->offset = (void*)(i*2*sizeof (uint16_t));
        dengine_shader_set_vec3(shader, "color", color);
        dengine_draw_primitive(axis, shader);
    }
}

DENGINE_EXPORT void dengitor_glarea_onrealize(GtkGLArea* area)
{
    Dengitor* dengitor = dengitor_get();

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
    opts->enable_logthread = 1;

    // preload glad to set viewport
    gladLoadGL();
    GtkAllocation glarea_alloc;
    gtk_widget_get_allocation(GTK_WIDGET(area), &glarea_alloc);
    glViewport(0, 0, glarea_alloc.width, glarea_alloc.height);

    dengine_init();
    dengitor_onevent_addcallback(dengitor_glearea_onevent);

    dengine_input_set_input(&dengitor->stdinput);

    /* we set up here since it uses log callback
     * which dengine_init ensures log thread runs */
    dengitor_log_setup(dengitor->builder, &dengitor->log);

    Camera camera;
    dengine_camera_setup(&camera);
    dengine_camera_set_rendermode(DENGINE_CAMERA_RENDER_FOWARD, &camera);
    camera.clearcolor[0] = 0.5f;
    camera.clearonuse = 0;

    denginescene_ecs_new_entity(&dengitor->scene_camera);
    denginescene_ecs_new_cameracomponent(&camera, &dengitor->scene_camera->camera_component);
    dengitor->scene_camera->transform.position[0] = 7.0f;
    dengitor->scene_camera->transform.position[1] = 7.0f;
    dengitor->scene_camera->transform.position[2] = 7.0f;
    dengitor->scene_camera->transform.rotation[0] = -45.0;
    dengitor->scene_camera->transform.rotation[1] = -135.0;

    Script scenecamera;
    denginescript_nsl_get_script("scenecamera", &scenecamera, dengitor->nsl_dengitor);
    denginescene_ecs_add_script(dengitor->scene_camera, &scenecamera);

    // compile some standard shaders
    dengine_shader_make_standard(DENGINE_SHADER_DEFAULT, &dengitor->shader_default);
    dengine_shader_make_standard(DENGINE_SHADER_STANDARD, &dengitor->shader_standard);
    dengine_shader_make_standard(DENGINE_SHADER_SHADOW2D, &dengitor->shader_shadow2d);
    dengine_shader_make_standard(DENGINE_SHADER_SHADOW3D, &dengitor->shader_shadow3d);
    dengine_shader_make_standard(DENGINE_SHADER_DEBUG_NORMALS, &dengitor->shader_debug_normals);
    dengine_shader_make_standard(DENGINE_SHADER_SKYBOXCUBE, &dengitor->shader_skybox_cube);
    dengine_shader_make_standard(DENGINE_SHADER_SKYBOX2D, &dengitor->shader_skybox_2d);
    dengine_shader_make_standard(DENGINE_SHADER_DEFAULT, &dengitor->shader_default);

    dengine_primitive_gen_grid(10, &dengitor->scene_grid, &dengitor->shader_default);
    dengine_primitive_gen_axis(&dengitor->scene_axis, &dengitor->shader_default);
    dengitor->scene_axis.index_count = 2;

    /* basic scene, no camera. test hierachy */
    denginescene_new(&dengitor->scene_current);

#if 1
    int hierlevel = 3;
    int i = 0;
    Entity *e, *c;
    denginescene_ecs_new_entity(&e);
    denginescene_add_entity(dengitor->scene_current, e);

    while(i < hierlevel)
    {
        denginescene_ecs_new_entity(&c);
        denginescene_ecs_parent(e, c);
        e = c;
        i++;
    }
    /*denginescene_dumphierachy_stdio(dengitor->scene_current, stderr);*/
#endif

#if 1
    void* mem;
    size_t memsz;
    // A SIMPLE SCENE
    Primitive cube;
    dengine_primitive_gen_cube(&cube, &dengitor->shader_standard);

    Material std_mat;
    Material sky_equireq;

    dengine_material_setup(&sky_equireq);
    dengine_material_set_shader_color(&dengitor->shader_skybox_2d, &sky_equireq);

    dengine_material_setup(&std_mat);
    dengine_material_set_shader_color(&dengitor->shader_standard, &std_mat);
    dengine_material_set_shader_shadow(&dengitor->shader_shadow2d, &std_mat);

    //skybox
    Texture equireq;
    memset(&equireq, 0, sizeof(Texture));
    equireq.type = GL_FLOAT;
    equireq.interface = DENGINE_TEXTURE_INTERFACE_FLOAT;
    if(!dengine_texture_issupprorted(GL_TEXTURE_2D, GL_FLOAT, GL_RGB, GL_RGB))
    {
        dengineutils_logging_log("WARNING::GPU has no float texture support! Falling back to 8-bit");
        equireq.interface = DENGINE_TEXTURE_INTERFACE_8_BIT;
        equireq.type = GL_UNSIGNED_BYTE;
    }
    equireq.filter_min = GL_LINEAR;
    equireq.format = GL_RGB;
    equireq.internal_format = GL_RGB;
    equireq.auto_dataonload = 1;
    dengine_load_asset("textures/hdri/sunset.hdr", &mem, &memsz);
    dengine_texture_load_mem(mem, memsz, 1, &equireq);
    free(mem);

    dengine_material_set_texture(&equireq, "eqireqMap", &sky_equireq);
    denginescene_new_skybox(&cube, &sky_equireq, &dengitor->scene_current->skybox);

    Entity* cube_ent ; denginescene_ecs_new_entity(&cube_ent);
    MeshComponent* cube_mesh ; denginescene_ecs_new_meshcomponent(&cube, &std_mat, &cube_mesh);
    cube_ent->mesh_component = cube_mesh;

    denginescene_add_entity(dengitor->scene_current, cube_ent);

    Primitive plane;
    dengine_primitive_gen_plane(&plane, &dengitor->shader_standard);

    Material plane_mat;
    dengine_material_setup(&plane_mat);
    dengine_material_set_shader_color(&dengitor->shader_standard, &plane_mat);
    dengine_material_set_shader_shadow(&dengitor->shader_shadow2d, &plane_mat);

    static const char* plane_tex_files_tgt[][2]=
    {
        {"brickwall.jpg", "diffuseTex"},
        {"brickwall_normal.jpg", "normalTex"}
    };

    Texture plane_tex;
    memset(&plane_tex, 0, sizeof(plane_tex));

    for(uint32_t i = 0; i < DENGINE_ARY_SZ(plane_tex_files_tgt); i++)
    {
        char prtbf[512];

        plane_tex.auto_dataonload=1;
        plane_tex.interface=DENGINE_TEXTURE_INTERFACE_8_BIT;
        snprintf(prtbf,sizeof(prtbf),"textures/2d/%s", plane_tex_files_tgt[i][0]);
        dengine_load_asset(prtbf, &mem, &memsz);
        dengine_texture_load_mem(mem, memsz, 1, &plane_tex);
        dengine_material_set_texture(&plane_tex, plane_tex_files_tgt[i][1], &plane_mat);
        free(mem);
    }

    Entity* plane_ent ; denginescene_ecs_new_entity(&plane_ent);
    MeshComponent* plane_mesh ; denginescene_ecs_new_meshcomponent(&plane, &plane_mat, &plane_mesh);
    plane_ent->mesh_component = plane_mesh;

    plane_ent->transform.position[1] = -1.0f;

    plane_ent->transform.scale[0] = 5.0f;
    plane_ent->transform.scale[1] = 5.0f;
    plane_ent->transform.scale[2] = 5.0f;

    Entity* ch ; denginescene_ecs_new_entity(&ch);
    MeshComponent* ch_mesh ; denginescene_ecs_new_meshcomponent(&cube, &std_mat, &ch_mesh);
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
        Entity* ch_l1 ; denginescene_ecs_new_entity(&ch_l1);
        g_snprintf(nm, sizeof(nm), "child lv %d", i);
        denginescene_ecs_set_entity_name(ch_l1, nm);
        denginescene_ecs_parent(ch, ch_l1);
        for(int j = 0; j < 3; j++)
        {
            Entity* ch_l2 ; denginescene_ecs_new_entity(&ch_l2);
            g_snprintf(nm, sizeof(nm), "child lv %d:%d", i, j);
            denginescene_ecs_set_entity_name(ch_l2, nm);
            denginescene_ecs_parent(ch_l1, ch_l2);
            if(i == 1)
            {
                for(int k = 0; k < 4; k++)
                {
                    Entity* ch_l3 ; denginescene_ecs_new_entity(&ch_l3);
                    g_snprintf(nm, sizeof(nm), "child lv %d:%d:%d", i, j, k);
                    denginescene_ecs_set_entity_name(ch_l3, nm);
                    denginescene_ecs_parent(ch_l2, ch_l3);
                }
            }
        }
    }

    denginescene_ecs_parent(cube_ent, ch);

    denginescene_add_entity(dengitor->scene_current, plane_ent);

    Camera camera2;
    dengine_camera_setup(&camera2);
    dengine_camera_set_rendermode(DENGINE_CAMERA_RENDER_FOWARD, &camera2);
    dengine_camera_resize(&camera2, 1280, 720);

    CameraComponent* cam2_comp ; denginescene_ecs_new_cameracomponent(&camera2, &cam2_comp);
    Entity* cam_ent ; denginescene_ecs_new_entity(&cam_ent);
    cam_ent->transform.position[0] = 7.0f;
    cam_ent->transform.position[1] = 5.0f;
    cam_ent->transform.rotation[0] = -45.0;
    cam_ent->transform.rotation[1] = -180.0;
    cam_ent->camera_component = cam2_comp;
    denginescene_ecs_set_entity_name(cam_ent, "this is a camera");
    denginescene_add_entity(dengitor->scene_current, cam_ent);

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

    LightComponent* light_comp ; denginescene_ecs_new_lightcomponent(DENGINE_LIGHT_DIR, &dLight, &light_comp);
    Entity* dLight_ent ; denginescene_ecs_new_entity(&dLight_ent);
    denginescene_ecs_set_entity_name(dLight_ent, "this is a dir light");
    dLight_ent->transform.position[0] = 3.0f;
    dLight_ent->transform.position[1] = 3.0f;
    dLight_ent->transform.position[2] = -3.0f;
    dLight_ent->light_component = light_comp;
    denginescene_add_entity(dengitor->scene_current, dLight_ent);

    PointLight pLight;
    memset(&pLight, 0, sizeof(PointLight));
    dengine_lighting_light_setup(DENGINE_LIGHT_POINT, &pLight);
    pLight.light.diffuse[0] = 0;
    denginescene_ecs_new_lightcomponent(DENGINE_LIGHT_POINT, &pLight, &light_comp);
    Entity* pLight_ent ; denginescene_ecs_new_entity(&pLight_ent);
    denginescene_ecs_set_entity_name(pLight_ent, "this is a point light");
    pLight_ent->light_component = light_comp;
    pLight_ent->transform.position[0] = -3.0f;
    pLight_ent->transform.position[1] = 3.0f;
    pLight_ent->transform.position[2] = 3.0f;
    denginescene_add_entity(dengitor->scene_current, pLight_ent);
#endif

    dengitor_scenetree_traverse(dengitor->scene_current, &dengitor->tree);
}


DENGINE_EXPORT void dengitor_glarea_onunrealize(GtkGLArea* area)
{
    Dengitor* dengitor = dengitor_get();
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
    denginescene_ecs_destroy_entity(dengitor->scene_camera);

    g_object_unref(dengitor->cursor_blank);
    g_object_unref(dengitor->cursor_arrow);

    if(dengitor->scene_current)
        denginescene_destroy(dengitor->scene_current);

    dengine_terminate();
}

DENGINE_EXPORT void dengitor_glarea_evbox_onmotion(GtkEventBox* evbox, GdkEventMotion* motion)
{
    Dengitor* dengitor = dengitor_get();
    StandardInput* stdinput = &dengitor->stdinput;
    GtkAllocation allo;
    gtk_widget_get_allocation(GTK_WIDGET(dengitor->glarea.glarea), &allo);
    stdinput->mouse_x = motion->x;
    stdinput->mouse_y = allo.height - motion->y;
    dengitor_redraw();
}


DENGINE_EXPORT void dengitor_glarea_evbox_onbtnpress(GtkEventBox* evbox, GdkEventButton* button)
{
    Dengitor* dengitor = dengitor_get();
    StandardInput* stdinput = &dengitor->stdinput;
    if(button->button == GDK_BUTTON_PRIMARY)
    {
        stdinput->mouse_btns[DENGINE_INPUT_MSEBTN_PRIMARY].state = 1;
        stdinput->mouse_btns[DENGINE_INPUT_MSEBTN_PRIMARY].button = DENGINE_INPUT_MSEBTN_PRIMARY;
    }else if(button->button == GDK_BUTTON_SECONDARY) {
        stdinput->mouse_btns[DENGINE_INPUT_MSEBTN_SECONDARY].state = 1;
        stdinput->mouse_btns[DENGINE_INPUT_MSEBTN_SECONDARY].button = DENGINE_INPUT_MSEBTN_SECONDARY;
    }else if (button->button == GDK_BUTTON_MIDDLE) {
        stdinput->mouse_btns[DENGINE_INPUT_MSEBTN_MIDDLE].state = 1;
        stdinput->mouse_btns[DENGINE_INPUT_MSEBTN_MIDDLE].button = DENGINE_INPUT_MSEBTN_MIDDLE;
    }
    if(button->button == GDK_BUTTON_SECONDARY && dengitor->glarea.glarea_mode == DENGITOR_GLAREA_MODE_SCENE)
    {
        /*GdkWindow* window = gtk_widget_get_window( GTK_WIDGET(dengitor->glarea.glarea) );*/
        /*gdk_window_set_cursor(window, dengitor->cursor_blank);*/
    }
    dengitor_glearea_onevent();
}

DENGINE_EXPORT void dengitor_glarea_evbox_onbtnrelease(GtkEventBox* evbox, GdkEventButton* button)
{
    Dengitor* dengitor = dengitor_get();
    StandardInput* stdinput = &dengitor->stdinput;
    if(button->button == GDK_BUTTON_PRIMARY)
    {
        memset(&stdinput->mouse_btns[DENGINE_INPUT_MSEBTN_PRIMARY], 0, sizeof(ButtonInfo));
    }else if(button->button == GDK_BUTTON_SECONDARY) {
        memset(&stdinput->mouse_btns[DENGINE_INPUT_MSEBTN_SECONDARY], 0, sizeof(ButtonInfo));
    }else if (button->button == GDK_BUTTON_MIDDLE) {
        memset(&stdinput->mouse_btns[DENGINE_INPUT_MSEBTN_MIDDLE], 0, sizeof(ButtonInfo));
    }
    GdkWindow* window = gtk_widget_get_window( GTK_WIDGET(dengitor->glarea.glarea) );
    gdk_window_set_cursor(window, dengitor->cursor_arrow);
}

DENGINE_EXPORT void dengitor_glarea_evbox_onkeypress(GtkEventBox* evbox, GdkEventKey* key)
{

}

DENGINE_EXPORT void dengitor_glarea_onrender(GtkGLArea* area)
{
    GdkGLContext* context = gtk_gl_area_get_context(area);
    Dengitor* dengitor = dengitor_get();
    gdk_gl_context_make_current(context);
    Texture* out = dengitor->scene_camera->camera_component->camera.framebuffer.color;

    // SCREEN QUAD PASS
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Framebuffer gdk_fb;
    dengine_entrygl_framebuffer(GL_FRAMEBUFFER, &gdk_fb);

    // SCENE CAMERA PASS
    int x, y, w, h;
    dengine_viewport_get(&x, &y, &w, &h);
    float aspect = (float)w / h;

    // process scene
    /*denginescene_update(dengitor->scene_current);*/

    Entity* current_ent = dengitor_get()->inspector.currententity;
    Entity* lastcam = NULL;

    if(dengitor->scene_current)
    {
        denginescene_update(dengitor->scene_current);
        lastcam = dengitor->scene_current->last_cam;
    }

    if(dengitor->glarea.glarea_mode == DENGITOR_GLAREA_MODE_SCENE)
    {


        Camera* scene_camera = &dengitor->scene_camera->camera_component->camera;
        dengine_camera_use(scene_camera);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        dengine_viewport_set(0, 0,
                             scene_camera->render_width, scene_camera->render_height);

        if(dengitor->scene_camera_last_w != w || dengitor->scene_camera_last_h != h)
        {
            dengine_camera_resize(scene_camera, w, h);
            dengitor->scene_camera_last_w = w;
            dengitor->scene_camera_last_h = h;
            dengineutils_logging_log("INFO::resize scene camera to %dx%d", w, h);dengitor->scene_camera_last_w = w;
        }

        dengine_camera_project_perspective( (float)w / (float)h,
                                            scene_camera);

        denginescene_ecs_transform_entity(dengitor->scene_camera);
        if(dengitor->scene_current)
            denginescene_ecs_do_camera_scene(dengitor->scene_camera, dengitor->scene_current);


        vec3 front;
        denginescene_ecs_get_front(dengitor->scene_camera, front);
        dengine_camera_lookat(front,
                              scene_camera);
        dengine_camera_apply(&dengitor->shader_default, scene_camera);

        static mat4 mat_4;
        static vec3 vec_3;
        float init_width;
        glGetFloatv(GL_LINE_WIDTH, &init_width);
        if(dengitor->scene_grid_draw)
        {
            glm_mat4_identity(mat_4);
            // DRAW grid
            vec_3[0] = dengitor->scene_grid_scale;
            vec_3[1] = dengitor->scene_grid_scale;
            vec_3[2] = dengitor->scene_grid_scale;

            glm_scale(mat_4, vec_3);
            dengine_shader_set_vec3(&dengitor->shader_default, "color", dengitor->scene_grid_color);
            dengine_shader_set_mat4(&dengitor->shader_default, "model",mat_4[0]);

            glLineWidth(dengitor->scene_grid_width);
            dengine_draw_primitive(&dengitor->scene_grid, &dengitor->shader_default);
        }

        glm_mat4_identity(mat_4);

        vec_3[0] = 0.0f;
        vec_3[1] = 0.01f;
        vec_3[2] = 0.0f;
        glm_translate(mat_4, vec_3);

        vec_3[0] = dengitor->scene_axis_scale;
        vec_3[1] = dengitor->scene_axis_scale;
        vec_3[2] = dengitor->scene_axis_scale;
        glm_scale(mat_4, vec_3);
        dengine_shader_set_mat4(&dengitor->shader_default, "model",mat_4[0]);



        // draw scene origin axis
        int dfunc;
        glGetIntegerv(GL_DEPTH_FUNC, &dfunc);

        // these axes will always be drawn on top of everything
        glDepthFunc(GL_ALWAYS);

        glLineWidth(dengitor->scene_axis_width);
        dengitor_glarea_draw_axis(&dengitor->scene_axis, &dengitor->shader_default);

        //draw a local axis for current entity
        if(current_ent)
        {
            glLineWidth(dengitor->scene_entity_current_axis_width);
            dengine_shader_set_mat4(&dengitor->shader_default, "model", current_ent->transform.world_model[0]);
            dengitor_glarea_draw_axis(&dengitor->scene_axis, &dengitor->shader_default);
        }
        glDepthFunc(dfunc);

        glLineWidth(init_width);

    }else
    {
        if(!dengitor->scene_current || !lastcam)
            out = NULL;
        else
            out = lastcam->camera_component->camera.framebuffer.color;
        aspect = (float)lastcam->camera_component->camera.render_width / lastcam->camera_component->camera.render_height;
    }

    dengine_viewport_set(x, y, w, h);
    dengine_framebuffer_bind(GL_FRAMEBUFFER, &gdk_fb);

    vec4 red = {1.0, 0.0, 0.0, 1.0};
    float pad = dengitor->viewportops.viewport_padding;
    if(!out)
        denginegui_text(10, (float)h / 2.0f, "NO CAMERA FOUND!", red);
    else
        denginegui_panel(pad, pad, (h * aspect) - (2.0f * pad), h - (2.0f * pad), out, NULL, GLM_VEC4_BLACK);

    denginegui_text(10, 10, (const char*)glGetString(GL_VERSION) , NULL);
}

DENGINE_EXPORT void dengitor_glarea_toggle_scene(GtkToggleButton* btn)
{
    Dengitor* dengitor = dengitor_get();
    dengitor->glarea.glarea_mode = DENGITOR_GLAREA_MODE_SCENE; 
    dengitor_redraw();
}
DENGINE_EXPORT void dengitor_glarea_toggle_game(GtkToggleButton* btn)
{
    Dengitor* dengitor = dengitor_get();
    dengitor->glarea.glarea_mode = DENGITOR_GLAREA_MODE_GAME;
    dengitor_redraw();
}
void dengitor_glarea_setup(GtkBuilder* builder, DengitorGLArea* glarea)
{
    // GL AREA, ALL THE MAGIC IS HERE!
    glarea->glarea = GTK_GL_AREA(gtk_builder_get_object(builder, "glarea"));
    glarea->glarea_evbox = GTK_EVENT_BOX( gtk_widget_get_parent( GTK_WIDGET(glarea->glarea) ) );
    glarea->toggle_game = GTK_RADIO_BUTTON(gtk_builder_get_object(builder, "toggle_game"));
    glarea->toggle_scene = GTK_RADIO_BUTTON(gtk_builder_get_object(builder, "toggle_scene"));
    gtk_radio_button_join_group(glarea->toggle_game, glarea->toggle_scene);
    glarea->glarea_mode = DENGITOR_GLAREA_MODE_SCENE;
}

