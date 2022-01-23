#include <android_native_app_glue.h>
#include <jni.h>
#include <android/asset_manager.h>

#include <dengine/window.h>
#include <dengine-utils/logging.h>
#include <dengine/loadgl.h>
#include <dengine-utils/timer.h>

#include <dengine/primitive.h>
#include <dengine/draw.h>
#include <dengine/shader.h>

#include <dengine/texture.h>

#include <dengine/camera.h>
#include <cglm/cglm.h>

#include <string.h> //memset
#include <stdlib.h> //malloc
#include <dengine-utils/filesys.h>
double elapsed;


Primitive cube;
Primitive plane;

Shader shader_cube;
Shader shader_plane;
AAssetManager* asset_mgr;

void* a2m(const char* file, size_t* sz)
{
    if(!asset_mgr)
        return NULL;
    AAsset* asset = AAssetManager_open(asset_mgr, file, AASSET_MODE_BUFFER);
    if(!asset)
        return NULL;

    *sz = AAsset_getLength(asset);

    //nullterm for txt
    void* mem = malloc(*sz + 1);
    memset(mem, 0, *sz + 1);
    AAsset_read(asset, mem, *sz);
    AAsset_close(asset);
    return mem;
}

static void init(struct android_app* app)
{
    //Acquire win
    ANativeWindow_acquire(app->window);
    dengine_window_android_set_nativewindow(app->window);

    dengine_window_request_GL(2, 0, 0);

    if(dengine_window_init())
    {
        int w, h;
        dengine_window_get_window_width(&w);
        dengine_window_get_window_height(&h);
        dengineutils_logging_log("INFO::init window %dx%d\n", w, h);

        //use fullscreen 60Hz on primary monitor
        //dengine_window_glfw_set_monitor(glfwGetPrimaryMonitor(), 0, 0, 60);

        dengineutils_logging_log("INFO::GL : %s\n", glGetString(GL_VERSION));
        shader_cube.vertex_code =
                "attribute vec3 aPos;"
                "attribute vec2 aTexCoord;"
                "attribute vec3 aNormal;"

                "uniform mat4 projection;"
                "uniform mat4 view;"
                "uniform mat4 model;"

                "varying vec2 TexCoord;"
                "varying vec3 Normal;"
                "varying vec3 FragPos;"

                "void main()"
                "{"
                "gl_Position =  projection * view * model * vec4(aPos, 1.0);"
                "TexCoord = aTexCoord;"
                "Normal = aNormal;"
                "FragPos = vec3(model * vec4(aPos, 1.0));"
                "}";
        shader_cube.fragment_code =
                "precision mediump float;"
                "varying vec2 TexCoord;"
                "varying vec3 Normal;"
                "varying vec3 FragPos;"

                "uniform vec3 ViewPos;"

                "uniform sampler2D texture;"

                "vec3 dirLight(vec3 dir, vec3 nNormal, vec3 viewDir){"
                "   float diff = max(dot(nNormal, -dir), 0.0);"
                "   vec3 diffuse = diff * vec3( texture2D(texture, TexCoord));"
                "   return diffuse;"
                "}"

                "void main()"
                "{"
                "vec3 nNormal = normalize(Normal);"
                "vec3 viewDir = normalize(ViewPos - FragPos);"
                "vec3 FragColor = vec3(0.0);"
                "vec3 lightDir = vec3(4.0, 5.0, 6.0);"
                "FragColor += dirLight(-normalize(lightDir), nNormal, viewDir);"
                "gl_FragColor = vec4(FragColor, 1.0);"
                "}";
        shader_plane.fragment_code = shader_cube.fragment_code;
        shader_plane.vertex_code = shader_cube.vertex_code;

        dengine_shader_create(&shader_cube);
        dengine_shader_setup(&shader_cube);

        dengine_shader_create(&shader_plane);
        dengine_shader_setup(&shader_plane);

        dengine_primitive_gen_cube(&cube, &shader_cube);
        dengine_primitive_gen_plane(&plane, &shader_plane);

        Camera camera;
        camera.near = 0.01f;
        camera.far = 100.0f;
        camera.fov = 60.0f;
        float distance = 3.0f;
        float position[] = {distance, distance, distance};
        float target[] = {0.0f, 0.0f, 0.0f};

        memcpy(camera.position, position, sizeof(position));

        //FIXME : Break on resize window framebuffer
        dengine_camera_project_perspective((float)w / (float)h, &camera);
        dengine_camera_lookat(target, &camera);

        mat4 model;
        glm_mat4_identity(model);
        vec3 cube_pos = {0.0f, 1.0f, 0.0f};
        glm_translate(model, cube_pos);

        dengine_shader_set_mat4(&shader_cube, "projection", camera.projection_mat);
        dengine_shader_set_mat4(&shader_cube, "view", camera.view_mat);
        dengine_shader_set_mat4(&shader_cube, "model", model[0]);
        dengine_shader_set_mat4(&shader_cube, "ViewPos", position);

        glm_mat4_identity(model);
        vec3 plane_scale = {5.0f, 5.0f, 5.0f};
        glm_scale(model, plane_scale);
        dengine_shader_set_mat4(&shader_plane, "projection", camera.projection_mat);
        dengine_shader_set_mat4(&shader_plane, "view", camera.view_mat);
        dengine_shader_set_mat4(&shader_plane, "model", model[0]);
        dengine_shader_set_mat4(&shader_plane, "ViewPos", position);

        size_t tex_sz;
        void* tex_mem = a2m("brickwall.jpg", &tex_sz);

        Texture texture;
        memset(&texture, 0, sizeof(Texture));
        texture.interface = DENGINE_TEXTURE_INTERFACE_8_BIT;

        dengine_texture_load_mem(tex_mem, tex_sz, 1, &texture);
        uint32_t fmt = texture.channels == 3 ? GL_RGB : GL_RGBA;
        texture.internal_format = fmt;
        texture.format = fmt;
        texture.type = GL_UNSIGNED_BYTE;

        dengine_texture_gen(1, &texture);
        //Dont unbind
        glActiveTexture(GL_TEXTURE0);
        dengine_texture_bind(GL_TEXTURE_2D, &texture);
        dengine_texture_data(GL_TEXTURE_2D, &texture);
        dengine_texture_set_params(GL_TEXTURE_2D, &texture);
        dengine_texture_free_data(&texture);

        dengine_shader_set_int(&shader_plane, "texture", 0);
        dengine_shader_set_int(&shader_cube, "texture", 0);

        //depth
        glEnable(GL_DEPTH_TEST);

        //face culling
        glEnable(GL_CULL_FACE);
    }
}

static void destroy(struct android_app* app)
{
    dengine_window_terminate();

    ANativeWindow_release(app->window);
}

static void draw()
{
    glClearColor(0.3, 0.2, 0.1, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    dengine_draw_primitive(&cube, &shader_cube);
    dengine_draw_primitive(&plane, &shader_plane);

    dengine_window_swapbuffers();
}

static void cmd_handle(struct android_app* app, int32_t cmd)
{
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            dengineutils_logging_log("Saving state...");
            break;

        case APP_CMD_INIT_WINDOW:
            dengineutils_logging_log("Getting window ready...");
            init(app);
            //Buggy fullscreen
            //ANativeActivity_setWindowFlags(app->activity, AWINDOW_FLAG_FULLSCREEN, 0);
            break;

        case APP_CMD_TERM_WINDOW:
            dengineutils_logging_log("Term window");
            destroy(app);
            break;

        case APP_CMD_GAINED_FOCUS:
            dengineutils_logging_log("Gained focus");
            break;

        case APP_CMD_LOST_FOCUS:
            dengineutils_logging_log("Lost focus");
            break;

        case APP_CMD_PAUSE:
            dengineutils_logging_log("Paused");
            break;

        case APP_CMD_RESUME:
            dengineutils_logging_log("Resumed");
            break;

        case APP_CMD_DESTROY:
            dengineutils_logging_log("Destroy");
            break;
    }
}

void android_main(struct android_app* state)
{
    //Set app callbacks
    state->onAppCmd = cmd_handle;
    asset_mgr = state->activity->assetManager;

    if(state->savedState)
    {
        dengineutils_logging_log("Restoring from save state");
    }

    while(1)
    {
        //Read events
        int events;
        struct android_poll_source* source;

        while((ALooper_pollAll(1 ? 0 : -1, NULL, &events, (void**)&source)) >= 0)
        {
            //Process event
            if(source != NULL)
            {
                source->process(state, source);
            }
        }

        //Quit and detach
        if(state->destroyRequested != 0)
        {
            dengineutils_logging_log("Destroy Requested");
            ANativeActivity_finish(state->activity);

            //state->activity->vm->DetachCurrentThread();

            dengineutils_logging_log("Goodbye!");
            return;
        }

        dengineutils_timer_update();

        //print every 1secs
        elapsed += dengineutils_timer_get_delta();
        if(elapsed > 5000.0)
        {
            dengineutils_logging_log("step");
            elapsed = 0;
        }

        draw();
    }
}