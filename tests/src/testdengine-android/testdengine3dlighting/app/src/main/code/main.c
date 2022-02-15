#include <dengine/android.h>
#include <dengine/window.h>
#include <dengine/loadgl.h>

#include <dengine-utils/logging.h>
#include <dengine-utils/timer.h>

#include <dengine/primitive.h>
#include <dengine/draw.h>
#include <dengine/texture.h>

#include <dengine/camera.h>
#include <cglm/cglm.h>

#include <string.h> //memset
#include <stdlib.h> //malloc
#include <dengine-utils/filesys.h>

double elapsed;

int window_init = 0;

Primitive cube;
Primitive plane;

mat4 model;

Shader shader;


static void init(struct android_app* app)
{
    //Acquire win
    ANativeWindow_acquire(app->window);
    dengine_window_android_set_nativewindow(app->window);

    dengine_window_request_GL(2, 0, 0);

    if(dengine_window_init())
    {
		
		dengine_window_loadgl();
		
		window_init = 1;
		
        int w, h;
        dengine_window_get_window_width(&w);
        dengine_window_get_window_height(&h);
        dengineutils_logging_log("INFO::init window %dx%d\n", w, h);

        //use fullscreen 60Hz on primary monitor
        //dengine_window_glfw_set_monitor(glfwGetPrimaryMonitor(), 0, 0, 60);

        dengineutils_logging_log("INFO::GL : %s\n", glGetString(GL_VERSION));
        shader.vertex_code =
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
        shader.fragment_code =
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
        

        dengine_shader_create(&shader);
        dengine_shader_setup(&shader);

        dengine_primitive_gen_cube(&cube, &shader);
        dengine_primitive_gen_plane(&plane, &shader);

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
		dengine_camera_apply(&shader, &camera);
        
        Texture texture;
        memset(&texture, 0, sizeof(Texture));
        texture.interface = DENGINE_TEXTURE_INTERFACE_8_BIT;
		
		File2Mem f2m;
        f2m.file = "brickwall.jpg";
        dengine_android_asset2file2mem(&f2m);
        dengine_texture_load_mem(f2m.mem, f2m.size, 1, &texture);
		
        uint32_t fmt = texture.channels == 3 ? GL_RGB : GL_RGBA;
        texture.internal_format = fmt;
        texture.format = fmt;
        texture.type = GL_UNSIGNED_BYTE;
		texture.mipmap = 1;

        dengine_texture_gen(1, &texture);
        //Dont unbind
        glActiveTexture(GL_TEXTURE0);
        dengine_texture_bind(GL_TEXTURE_2D, &texture);
        dengine_texture_data(GL_TEXTURE_2D, &texture);
		
        dengine_texture_set_params(GL_TEXTURE_2D, &texture);
        dengine_texture_free_data(&texture);
		dengineutils_filesys_file2mem_free(&f2m);

        dengine_shader_set_int(&shader, "texture", 0);
		glActiveTexture(GL_TEXTURE0);

        //depth
        glEnable(GL_DEPTH_TEST);

        //face culling
        glEnable(GL_CULL_FACE);
    }
}

static void term(struct  android_app* app)
{
    dengine_window_terminate();
    ANativeWindow_release(app->window);
    ANativeActivity_finish(app->activity);
}

static void draw()
{
    glClearColor(0.3, 0.2, 0.1, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm_mat4_identity(model);
	vec3 cube_pos = {0.0f, 1.0f, 0.0f};
	glm_translate(model, cube_pos);
	dengine_shader_set_mat4(&shader, "model", model[0]);
	dengine_draw_primitive(&cube, &shader);

	glm_mat4_identity(model);
	vec3 plane_scale = {5.0f, 5.0f, 5.0f};
	glm_scale(model, plane_scale);
	dengine_shader_set_mat4(&shader, "model", model[0]);
    dengine_draw_primitive(&plane, &shader);

    dengine_window_swapbuffers();
}

void android_main(struct android_app* state)
{
    dengine_android_set_app(state);
    dengine_android_set_initfunc(init);
    dengine_android_set_terminatefunc(term);

    if(state->savedState)
    {
        dengineutils_logging_log("Restoring from save state");
    }

    while(1)
    {
        dengine_android_pollevents();
		
        //Quit and detach
        if(state->destroyRequested != 0)
        {
            dengineutils_logging_log("Destroy Requested");
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
		if(window_init)
        	draw();
    }
}