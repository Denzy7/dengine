#include "dengine-gui/gui.h"

#include "dengine/texture.h"
#include "dengine/loadgl.h"
#include "dengine/primitive.h"
#include "dengine/shader.h"
#include "dengine/draw.h"
#include "dengine/window.h"
#include "dengine/input.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h> //stbtt

#include <cglm/cam.h> //ortho

#include <stdlib.h> //malloc
#include <string.h> //memset
#include <stdio.h>  //printf

stbtt_packedchar packedchar_data[96];
stbtt_fontinfo info;
//baked fontmap
Texture fontmap;
unsigned int _bmp_sz = 0;
int initfont = 0;
float _fontsz = 0.0f;

//the standard gui quad
Primitive quad;
Shader shader;

int initgui = 0;
#define PANEL_ALPHA 0.4f

int denginegui_init()
{
    static float vertices[16];

    static uint16_t indices[]=
    {
        0, 3, 2, 2, 1, 0
    };

    quad.draw_mode = GL_TRIANGLES;
    quad.draw_type = GL_UNSIGNED_SHORT;

    quad.array.data = vertices;
    quad.array.size = sizeof(vertices);
    quad.array.usage = GL_DYNAMIC_DRAW;

    quad.index.data = indices;
    quad.index.size = sizeof(indices);
    quad.index.usage = GL_STATIC_DRAW;
    quad.index_count = sizeof(indices) / sizeof(indices[0]);

    quad.aPos.size = 2;
    quad.aPos.stride = 4 * sizeof(float);
    quad.aPos.type = GL_FLOAT;
    quad.aPos.ptr = NULL;

    quad.aTexCoord.size = 2;
    quad.aTexCoord.stride = 4 * sizeof(float);
    quad.aTexCoord.type = GL_FLOAT;
    quad.aTexCoord.ptr = (void*)(2 * sizeof(float));

    shader.vertex_code =
            "#version 100\n"
            "attribute vec2 aPos;"
            "attribute vec2 aTexCoord;"
            "varying vec2 TexCoord;"
            "uniform mat4 projection;"
            "void main()"
            "{"
                "gl_Position = projection * vec4(aPos, 0.0, 1.0);"
                "TexCoord = aTexCoord;"
            "}";
    shader.fragment_code =
            "#version 100\n"
            "precision mediump float;"
            "varying vec2 TexCoord;"
            "uniform sampler2D tex;"
            "uniform vec4 col;"
            "uniform int istext;"
            "void main()"
            "{"
                "vec4 a = vec4(0.0);"
                "if(istext == 1)"
                "   a = vec4(1.0, 1.0, 1.0, texture2D(tex, TexCoord).a) * col;"
                "else"
                "   a = vec4( texture2D(tex, TexCoord).rgb + col.rgb, col.a);"
                "gl_FragColor = a;"
            "}";

    dengine_shader_create(&shader);
    if(dengine_shader_setup(&shader))
    {
        dengine_shader_set_int(&shader, "tex", 0);
        dengine_primitive_setup(&quad, &shader);
        initgui = 1;
        return 1;
    }else
    {
        printf("cannot init quad shader\n");
        return 0;
    }
}

int denginegui_set_font(void* ttf, const float fontsize, unsigned int bitmap_size)
{
    if(!stbtt_InitFont(&info, ttf, 0))
    {
        printf("failed to init font\n");
        return 0;
    }

    unsigned char baked_bmp[bitmap_size * bitmap_size];

    stbtt_pack_context ctx;
    if(!stbtt_PackBegin(&ctx, baked_bmp, bitmap_size, bitmap_size, 0, 1, NULL))
    {
        printf("failed to pack chars\n");
        return 0;
    }

    if(!stbtt_PackFontRange(&ctx, ttf, 0, fontsize, 32, 96, packedchar_data))
    {
        printf("failed to pack chars range\n");
        return 0;
    }
    stbtt_PackEnd(&ctx);

    if(!initfont)
        memset(&fontmap, 0, sizeof(Texture));

    fontmap.format = GL_ALPHA;
    fontmap.internal_format = GL_ALPHA;
    fontmap.type = GL_UNSIGNED_BYTE;

    fontmap.data = baked_bmp;
    fontmap.filter_min = GL_LINEAR;

    fontmap.width = bitmap_size;
    fontmap.height = bitmap_size;

    if(!initfont)
    {
        dengine_texture_gen(1, &fontmap);
        initfont = 1;
    }

    _bmp_sz = bitmap_size;
    dengine_texture_bind(GL_TEXTURE_2D, &fontmap);
    dengine_texture_data(GL_TEXTURE_2D, &fontmap);
    dengine_texture_set_params(GL_TEXTURE_2D, &fontmap);
    dengine_texture_bind(GL_TEXTURE_2D, NULL);

    _fontsz = fontsize;

    return 1;
}

void _denginegui_projectquad()
{
    mat4 projection;
    int w, h;
    dengine_window_get_window_height(&h);
    dengine_window_get_window_width(&w);
    glm_ortho(0.0, w, 0.0, h, -1.0f, 1.0f, projection);
    dengine_shader_set_mat4(&shader, "projection", projection[0]);
}

void _denginegui_drawquad()
{
    dengine_shader_use(&shader);

    //return previous src, dst alpha and blend
    int srcalpha, dstalpha;
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &srcalpha); DENGINE_CHECKGL;
    glGetIntegerv(GL_BLEND_DST_ALPHA, &dstalpha); DENGINE_CHECKGL;
    int blnd;
    glGetIntegerv(GL_BLEND, &blnd); DENGINE_CHECKGL;

    if(!blnd)
        glEnable(GL_BLEND); DENGINE_CHECKGL;

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); DENGINE_CHECKGL;

    dengine_draw_primitive(&quad, &shader);

    glBlendFunc(srcalpha, dstalpha); DENGINE_CHECKGL;

    if(!blnd)
        glDisable(GL_BLEND); DENGINE_CHECKGL;

    dengine_shader_use(NULL);
}

void denginegui_text(float x, float y, const char* text, float* rgba)
{
    if(!initgui || !initfont)
        return;

    _denginegui_projectquad();

    for(size_t i = 0; i < strlen(text); i++)
    {
        if(text[i] >= 32 && text[i] < 127)
        {
            stbtt_aligned_quad q;
            char char_idx = text[i] - 32;
            stbtt_GetPackedQuad(packedchar_data, _bmp_sz, _bmp_sz, char_idx, &x, &y, &q, 1);

            float w = (q.x1 - q.x0);
            float h = (q.y1 - q.y0);

            float ds = q.s1 - q.s0;
            float dt = q.t1 - q.t0;

            float y_off = 2 * (q.y1 - y);
            float vertices[]=
            {
              //normal x, y
        //        q.x0, q.y1, q.s0, q.t0,
        //        q.x0, q.y1 + h, q.s0, q.t0 + dt,
        //        q.x0 + w, q.y1 + h, q.s0 + ds, q.t0 + dt,
        //        q.x0 + w, q.y1, q.s0 + ds, q.t0,

                //invert y??
                q.x0, q.y1 - y_off, q.s0, q.t0 + dt,
                q.x0, q.y1 + h - y_off, q.s0, q.t0,
                q.x0 + w, q.y1 + h - y_off, q.s0 + ds, q.t0,
                q.x0 + w, q.y1 - y_off, q.s0 + ds, q.t0 + dt
            };
            float white[4] = {1.0f, 1.0f, 1.0f, 1.0f};

            dengine_shader_set_int(&shader, "istext", 1);

            if(!rgba)
                dengine_shader_set_vec4(&shader, "col", white);
            else
                dengine_shader_set_vec4(&shader, "col", rgba);


            quad.array.data = vertices;

            dengine_buffer_bind(GL_ARRAY_BUFFER, &quad.array);
            dengine_buffer_data(GL_ARRAY_BUFFER, &quad.array);
            dengine_buffer_bind(GL_ARRAY_BUFFER, NULL);

            dengine_texture_bind(GL_TEXTURE_2D, &fontmap);

            _denginegui_drawquad();

            dengine_texture_bind(GL_TEXTURE_2D, NULL);
        }
    }
}

void denginegui_panel(float x, float y, float width, float height, Texture* texture, float* uv, float* rgba)
{
    if(!initgui)
        return;

    _denginegui_projectquad();

    float vertices[]=
    {
                         //Vert //TexCoord
                x,           y, 0.0f, 0.0f,
                x,  y + height, 0.0f, 1.0f,
        x + width,  y + height, 1.0f, 1.0f,
        x + width,           y, 1.0f, 0.0f,
    };

    quad.array.data = vertices;

    dengine_buffer_bind(GL_ARRAY_BUFFER, &quad.array);
    dengine_buffer_data(GL_ARRAY_BUFFER, &quad.array);
    dengine_buffer_bind(GL_ARRAY_BUFFER, NULL);

    float white[4] = {0.0f, 0.0f, 0.0f, PANEL_ALPHA};

    if(!rgba)
        dengine_shader_set_vec4(&shader, "col", white);
    else
        dengine_shader_set_vec4(&shader, "col", rgba);

    dengine_shader_set_int(&shader, "istext", 0);

    if(texture)
        dengine_texture_bind(GL_TEXTURE_2D, texture);

    _denginegui_drawquad();

    dengine_texture_bind(GL_TEXTURE_2D, NULL);

}

int denginegui_button(float x,float y, float width, float height, const char* text, float* rgba)
{
    int down = 0;
    double mx = dengine_input_get_mousepos_x();
    double my = dengine_input_get_mousepos_y();

    if(mx >= x && mx <= (x + width) &&
            my >= y && my <= (y + height) )
    {
        float hover[4] = {0.2f, 0.2f, 0.2f, PANEL_ALPHA};
        float press[4] = {0.02f, 0.02f, 0.02f, PANEL_ALPHA};
        if(rgba)
        {
            for(int i = 0; i < 3; i++)
                hover[i] += rgba[i];
            hover[3] = rgba[3];
        }else
        {
            for(int i = 0; i < 3; i++)
                hover[i] += 0.01f;
        }

        if(dengine_input_get_mousebtn(0))
        {
            if(rgba)
            {
                for(int i = 0; i < 3; i++)
                    press[i] += rgba[i];
                press[3] = rgba[3];
            }else
            {
                for(int i = 0; i < 3; i++)
                    press[i] += 0.005f;
            }
            denginegui_panel(x, y, width, height, NULL, NULL, press);
        }else{
            denginegui_panel(x, y, width, height, NULL, NULL, hover);
        }

        if(dengine_input_get_mousebtn_once(0))
            down = 1;
    }else
    {
        denginegui_panel(x, y, width, height, NULL, NULL, rgba);
    }
    float txtoffst = (height / 2) - (_fontsz / 4);
    denginegui_text(x + (txtoffst / 2), y + txtoffst, text, NULL);

    return down;
}
