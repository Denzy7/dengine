#include <dengine/window.h> //window_init, create
#include <dengine/loadgl.h> //gl
#include <dengine/shader.h> //shader
#include <dengine-utils/logging.h>//log
#include <dengine/primitive.h> //gen_plane
#include <dengine/draw.h> //draw_prim
#include <dengine/camera.h> //camera
#include <dengine/texture.h>

#include <dengine/lighting.h>

#include <dengine-utils/os.h> //filedialogopen
#include <dengine/input.h>
#include <string.h> //memset, memcpy
#include <cglm/cglm.h>      //mat4

#include <dengine-utils/filesys.h> //f2m
#include <dengine-gui/gui.h> //text
#include <dengine-utils/timer.h> //delta
int main(int argc, char** argv)
{
    int ctx32 = 1;
    dengine_window_init();
    //we need at least a 3.2 context for glFramebufferTexture (and GLSL 150 for GEOM shader)...
    //for shadow cubemap.
    //a core context is not a must AFAIK, but set to 1 if crash
    dengine_window_request_GL(3, 2, 0);
    if(!dengine_window_glfw_create(1280, 720, "testdengine-spotlight"))
    {
        dengineutils_logging_log("WARNING::cannot request an OpenGL 3.2 window. Shadows disabled\n");

        //Too bad we can't have 3.2
        //Use 3.0 then without shadows
        dengine_window_request_GL(3, 0, 0);
        ctx32 = 0;

        if(!dengine_window_glfw_create(1280, 720, "testdengine-pointlight(noshadow)"))
        {
            dengineutils_logging_log("WARNING::cannot request an OpenGL 3.0 window!");
            return 1;
        }
    }

    GLFWwindow* current = dengine_window_glfw_get_currentwindow();
    dengine_window_glfw_context_makecurrent(current);

    if(!dengine_window_glfw_context_gladloadgl())
    {
        dengineutils_logging_log("ERROR::cannot load gl!\n");
        return 1;
    }

    int w, h;
    dengine_window_get_window_width(&w);
    dengine_window_get_window_height(&h);
    dengineutils_logging_log("INFO::init window %dx%d\n", w, h);

    //use fullscreen 60Hz on primary monitor
    //dengine_window_glfw_set_monitor(glfwGetPrimaryMonitor(), 0, 0, 60);
    dengineutils_logging_log("INFO::GL : %s\n", glGetString(GL_VERSION));
    Shader shader;
    shader.vertex_code =
            "#version 100\n"
            "attribute vec3 aPos;"
            "attribute vec2 aTexCoord;"
            "attribute vec3 aNormal;"

            "uniform mat4 projection;"
            "uniform mat4 view;"
            "uniform mat4 model;"
            "uniform mat4 ivTpModel;"

            "varying vec2 TexCoord;"
            "varying vec3 Normal;"
            "varying vec3 FragPos;"

            "void main()"
            "{"
                "gl_Position =  projection * view * model * vec4(aPos, 1.0);"
                "TexCoord = aTexCoord;"
                "Normal = mat3(ivTpModel) * aNormal;"
                "FragPos = vec3(model * vec4(aPos, 1.0));"
            "}";
    shader.fragment_code =
            "#version 100\n"
            "precision mediump float;"
            "varying vec2 TexCoord;"
            "varying vec3 Normal;"
            "varying vec3 FragPos;"

            "uniform int ctx32;"

            //Spotlight
            "uniform float constant, linear, quadratic;"
            "uniform vec3 lightPos;"
            "uniform vec3 lightDir;"
            "uniform float oCut;"
            "uniform float iCut;"
            "uniform vec3 diffuseCol;"
            "uniform samplerCube shadowmap;"
            "uniform float shadowfar;"

            "uniform vec3 ViewPos;"

            "uniform sampler2D texture;"

            "float shadow_bias = 0.0;"

            "float shadowCalc3D(vec3 pos)"
            "{"
                "float shadow = 0.0;"
                "vec3 distance = vec3(0.0);"
                "distance = FragPos - pos;"

                "float current = length(distance);"
                "float close = textureCube(shadowmap, distance).r;"
                //return to depth
                "close*=shadowfar;"
                "shadow = current - shadow_bias > close ? 1.0 : 0.0;"
                "return shadow;"
            "}"

            "vec3 spotLight(vec3 pos, vec3 dir_u, vec3 nNormal, vec3 viewDir){"
                "vec3 dir = normalize(pos - FragPos);"
                "float diff = max(dot(nNormal, dir), 0.0);"
                "vec3 diffuse = diffuseCol * diff * vec3( texture2D(texture, TexCoord));"
                "float theta = dot(dir, normalize(-dir_u));"
                "float epsilon = iCut - oCut;"
                "float intensity = clamp( (theta - oCut) / epsilon, 0.0, 1.0);"
                "diffuse*=intensity;"
                "float shadow = 0.0;"
                "shadow_bias = max(0.01 * (1.0 - dot(nNormal, dir)), 0.005);"
                "if(ctx32 == 1)"
                "{"
                    "shadow = shadowCalc3D(pos);"
                "}"
                "float distance = length(pos - FragPos);"
                "float atten = 1.0 / (constant + linear * distance + quadratic * (distance * distance));"
                "return atten * diffuse * (1.0 - shadow);"
            "}"

            "void main()"
            "{"
                "vec3 nNormal = normalize(Normal);"
                "vec3 viewDir = normalize(ViewPos - FragPos);"

                "vec3 FragColor = vec3(0.0);"
                "FragColor += spotLight(lightPos, lightDir, nNormal, viewDir);"
                "gl_FragColor = vec4(FragColor, 1.0);"
            "}";

    Shader shadow;
    dengine_shader_create(&shadow);
    shadow.vertex_code=
            "#version 150\n"
            "attribute vec3 aPos;"

            "uniform mat4 model;"
            "void main()"
            "{"
                "gl_Position = model * vec4(aPos, 1.0);"
            "}";
    shadow.geometry_code=
            "#version 150\n"
            "layout (triangles) in;"
            "layout (triangle_strip, max_vertices = 18) out;"

            "uniform mat4 matrices[6];"

            "out vec4 FragPos;"

            "void main()"
            "{"
                "for(int face = 0; face < 6; face++)"
                "{"
                    "gl_Layer = face;"
                    "for(int i = 0; i < 3; i++)"
                    "{"
                        "FragPos = gl_in[i].gl_Position;"
                        "gl_Position = matrices[face] * FragPos;"
                        "EmitVertex();"
                    "}"
                    "EndPrimitive();"
                "}"
            "}";
    shadow.fragment_code=
            "#version 150\n"
            "in vec4 FragPos;"

            "uniform vec3 pos;"
            "uniform float far;"

            "void main()"
            "{"
                "float distance = length(FragPos.xyz - pos);"
                "distance /= far;"
                "gl_FragDepth = distance;"
            "}";

    if(ctx32)
        dengine_shader_setup(&shadow);

    dengine_shader_create(&shader);
    dengine_shader_setup(&shader);

    dengine_shader_set_int(&shader, "ctx32", ctx32);
    dengine_shader_set_int(&shader, "texture", 0);
    dengine_shader_set_int(&shader, "shadowmap", 1);

    Shader sLightGizmo;
    sLightGizmo.vertex_code=
            "#version 100\n"
            "attribute vec3 aPos;"
            "uniform mat4 projection;"
            "uniform mat4 view;"
            "uniform mat4 model;"
            "void main()"
            "{"
                "gl_Position = projection * view * model * vec4(aPos, 1.0);"
            "}";
    sLightGizmo.fragment_code =
            "#version 100\n"
            "precision mediump float;"
            "uniform vec3 color;"
            "void main()"
            "{"
                "gl_FragColor = vec4(color, 1.0);"
            "}";

    dengine_shader_create(&sLightGizmo);
    dengine_shader_setup(&sLightGizmo);

    SpotLight sLight;
    memset(&sLight, 0, sizeof(SpotLight));
    sLight.pointLight.shadow.enable = 1;
    sLight.pointLight.shadow.shadow_map_size = 512;
    float x = 0.f, y = 5.f, z = 2.5f;
    sLight.pointLight.position[0] = x;
    sLight.pointLight.position[1] = y;
    sLight.pointLight.position[2] = z;

    float tgt[3] = {1.5f,0.3f,0.f};

    dengine_lighting_setup_spotlight(&sLight);
    sLight.pointLight.light.strength = 1.0f;

    Texture texture;
    memset(&texture, 0, sizeof(Texture));
    texture.interface = DENGINE_TEXTURE_INTERFACE_8_BIT;
    char* texfile = dengineutils_os_dialog_fileopen("Open a 3/4 channel image(png, jpg)");
    if(!texfile)
    {
        dengineutils_logging_log("ERROR::no file selected!");
        return 1;
    }
    dengine_texture_load_file(texfile, 1, &texture);
    free(texfile);
    uint32_t fmt = texture.channels == 3 ? GL_RGB : GL_RGBA;
    texture.internal_format = fmt;
    texture.format = fmt;
    texture.type = GL_UNSIGNED_BYTE;
    texture.mipmap = 1;

    dengine_texture_gen(1, &texture);
    dengine_texture_bind(GL_TEXTURE_2D, &texture);
    dengine_texture_data(GL_TEXTURE_2D, &texture);
    dengine_texture_set_params(GL_TEXTURE_2D, &texture);
    dengine_texture_free_data(&texture);
    dengine_texture_bind(GL_TEXTURE_2D, NULL);

    Primitive cube;
    dengine_primitive_gen_cube(&cube, &shader);

    Primitive plane;
    dengine_primitive_gen_plane(&plane, &shader);

    Primitive axis, quad;
    dengine_primitive_gen_quad(&quad, &sLightGizmo);
    dengine_primitive_gen_axis(&axis, &sLightGizmo);
    axis.index_count = 2;

    Camera camera;
    camera.near = 0.01f;
    camera.far = 100.0f;
    camera.fov = 60.0f;
    float distance = 7.0f;
    float position[] = {distance, distance, distance};
    float target[] = {0.0f, 0.0f, 0.0f};

    memcpy(camera.position, position, sizeof(position));

    //"wirframe" mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    //3d depth
    glEnable(GL_DEPTH_TEST);

    dengine_input_init();

    //Change line size
    glLineWidth(4.0f);
    float color[3];
    float scale_fac = 0.4f;
    vec3 scale_gizmo = {scale_fac , scale_fac , scale_fac }, up = {0.f,1.f,0.f};

    mat4 view;

    denginegui_init();
    float fontsz = 32.f;
    denginegui_set_font(NULL, fontsz,512);

    char fps[20];
    double elapsed = 0;
    snprintf(fps, sizeof (fps), "FPS : ...");

    char* messages[] =
    {
        "Use 1-6 to change RGB, WASD - move light, EC - up/down",
        "Use YGHJ-move target, UM - up/down",
        "Use [ or ] inner cone, - or = inner cone"
    };

    while(dengine_window_isrunning())
    {
        dengine_camera_project_perspective((float)w / (float)h, &camera);
        dengine_camera_lookat(target, &camera);

        dengine_camera_apply(&shader, &camera);
        dengine_camera_apply(&sLightGizmo, &camera);

        double srcl = dengine_input_get_mousescroll_y();

        if(srcl > 0)
            camera.fov-=1;
        else if (srcl < 0)
            camera.fov+=1;

        //Change position

        if(dengine_input_get_key('D'))
            sLight.pointLight.position[0] += 0.1f;

        if(dengine_input_get_key('A'))
            sLight.pointLight.position[0] -= 0.1f;

        if(dengine_input_get_key('W'))
            sLight.pointLight.position[2] -= 0.1f;

        if(dengine_input_get_key('S'))
            sLight.pointLight.position[2] += 0.1f;

        if(dengine_input_get_key('E'))
            sLight.pointLight.position[1] += 0.1f;

        if(dengine_input_get_key('C'))
            sLight.pointLight.position[1] -= 0.1f;

        //Change diffuse, r, g, b
        if(dengine_input_get_key('1'))
            sLight.pointLight.light.diffuse[0] += 0.01f;

        if(dengine_input_get_key('2'))
            sLight.pointLight.light.diffuse[0] -= 0.01f;

        if(dengine_input_get_key('3'))
            sLight.pointLight.light.diffuse[1] += 0.01f;

        if(dengine_input_get_key('4'))
            sLight.pointLight.light.diffuse[1] -= 0.01f;

        if(dengine_input_get_key('5'))
            sLight.pointLight.light.diffuse[2] += 0.01f;

        if(dengine_input_get_key('6'))
            sLight.pointLight.light.diffuse[2] -= 0.01f;


        //Target positioning
        if(dengine_input_get_key('Y'))
            tgt[2] -= 0.1f;

        if(dengine_input_get_key('H'))
            tgt[2] += 0.1f;

        if(dengine_input_get_key('G'))
            tgt[0] -= 0.1f;

        if(dengine_input_get_key('J'))
            tgt[0] += 0.1f;

        if(dengine_input_get_key('U'))
            tgt[1] += 0.1f;

        if(dengine_input_get_key('M'))
            tgt[1] -= 0.1f;

        if(dengine_input_get_key('='))
            sLight.innerCutOff += .1f;

        if(dengine_input_get_key('-'))
            sLight.innerCutOff -= .1f;

        if(dengine_input_get_key('['))
            sLight.outerCutOff -= .1f;

        if(dengine_input_get_key(']'))
            sLight.outerCutOff += .1f;

        sLight.direction[0] = (tgt[0] - sLight.pointLight.position[0]);
        sLight.direction[1] = (tgt[1] - sLight.pointLight.position[1]);
        sLight.direction[2] = (tgt[2] - sLight.pointLight.position[2]);

        glClearColor(0.1, 0.1, 0.1, 0.1);
        //clear depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(ctx32)
            dengine_lighting_shadowop_clear(&sLight.pointLight.shadow);

        glActiveTexture(GL_TEXTURE0);
        dengine_texture_bind(GL_TEXTURE_2D, &texture);

        glActiveTexture(GL_TEXTURE1);
        dengine_texture_bind(GL_TEXTURE_CUBE_MAP, &sLight.pointLight.shadow.shadow_map.depth);

        mat4 model, ivTpModel;

        dengine_lighting_apply_spotlight(&sLight, &shader);

        //Cull faces for meshes
        glEnable(GL_CULL_FACE);

        glm_mat4_identity(model);
        vec3 pos = {0.0, 1.0, 0.0};
        glm_translate(model, pos);
        glm_mat4_inv(model, ivTpModel);
        glm_mat4_transpose(ivTpModel);
        dengine_shader_set_mat4(&shader, "model", model[0]);
        dengine_shader_set_mat4(&shader, "ivTpModel", ivTpModel[0]);
        dengine_draw_primitive(&cube, &shader);

        if(ctx32)
            dengine_lighting_shadow_spotlight_draw(&sLight, &shadow, &cube, model[0]);

        vec3 scale = {5.0, 5.0, 5.0};
        glm_mat4_identity(model);
        glm_scale(model, scale);
        glm_mat4_inv(model, ivTpModel);
        glm_mat4_transpose(ivTpModel);
        dengine_shader_set_mat4(&shader, "model", model[0]);
        dengine_shader_set_mat4(&shader, "ivTpModel", ivTpModel[0]);
        dengine_draw_primitive(&plane, &shader);

        if(ctx32)
            dengine_lighting_shadow_spotlight_draw(&sLight, &shadow, &cube, model[0]);

        //GIZMOS
        //Disable for gizmos
        glDisable(GL_CULL_FACE);
        glm_mat4_identity(model);
        glm_mat4_zero(view);
        glm_look(sLight.pointLight.position, sLight.direction, up, view);
        glm_mat4_inv(view, view);
        glm_mat4_mul(model, view, model);

        glm_scale(model, scale_gizmo);
        dengine_shader_set_mat4(&sLightGizmo, "model", model[0]);

        //AXIS
        for (int i = 0; i < 3; i++) {
            color[0] = i == 0 ? 1.0f : 0.0f, color[1] = i == 1 ? 1.0f : 0.0f, color[2] = i == 2 ? 1.0f : 0.0f;
            axis.offset = (void*)(i*2*sizeof (uint16_t));
            dengine_shader_set_vec3(&sLightGizmo, "color", color);
            dengine_draw_primitive(&axis, &sLightGizmo);
        }

        //Quad
        dengine_shader_set_vec3(&sLightGizmo, "color", sLight.pointLight.light.diffuse);
        dengine_draw_primitive(&quad, &sLightGizmo);

        //Target
        glm_mat4_identity(model);
        glm_translate(model, tgt);
        glm_scale(model, scale_gizmo);
        dengine_shader_set_mat4(&sLightGizmo, "model", model[0]);
        for (int i = 0; i < 3; i++) {
            color[0] = i == 0 ? 1.0f : 0.0f, color[1] = i == 1 ? 1.0f : 0.0f, color[2] = i == 2 ? 1.0f : 0.0f;
            axis.offset = (void*)(i*2*sizeof (uint16_t));
            dengine_shader_set_vec3(&sLightGizmo, "color", color);
            dengine_draw_primitive(&axis, &sLightGizmo);
        }

        for (int i = 0 ; i < sizeof (messages) / sizeof (messages[0]); i++) {
            denginegui_text(10, 10 + i * fontsz, messages[i], NULL);
        }

        dengineutils_timer_update();
        double delta = dengineutils_timer_get_delta();
        elapsed+=delta;
        if (elapsed > 1000) {
            snprintf(fps, sizeof (fps), "FPS : %.1f", 1 / delta * 1000);
            elapsed = 0;
        }

        denginegui_text(sizeof (fps), h - fontsz, fps, NULL);

        dengine_window_swapbuffers();
        dengine_window_glfw_pollevents();
    }

    dengine_window_terminate();
    return 0;
}
