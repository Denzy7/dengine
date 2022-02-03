#include "dengine-gui/gui.h"

#include "dengine/texture.h"
#include "dengine/loadgl.h"
#include "dengine/primitive.h"
#include "dengine/shader.h"
#include "dengine/draw.h"
#include "dengine/window.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h> //stbtt

#include <cglm/cam.h> //ortho

#include <stdlib.h> //malloc
#include <string.h> //memset
#include <stdio.h>  //printf

stbtt_packedchar packedchar_data[96];

//baked fontmap
Texture fontmap;
unsigned int _bmp_sz = 0;
int initfont = 0;

//the standard gui quad
Primitive quad;
Shader shader;

int initgui = 0;
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
            "varying vec2 TexCoord;"
            "uniform sampler2D tex;"
            "uniform vec3 col;"
            "void main()"
            "{"
                "vec4 a = vec4(1.0, 1.0, 1.0, texture2D(tex, TexCoord).a);"
                "gl_FragColor = a * vec4(col, 1.0);"
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
    stbtt_fontinfo info;
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

    return 1;
}
void denginegui_text(float x, float y, const char* text, float* rgb)
{
    if(!initgui || !initfont)
        return;

    mat4 projection;
    int w, h;
    dengine_window_get_window_height(&h);
    dengine_window_get_window_width(&w);
    glm_ortho(0.0, w, 0.0, h, -1.0f, 1.0f, projection);
    dengine_shader_set_mat4(&shader, "projection", projection[0]);

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
            float white[3] = {1.0f, 1.0f, 1.0f};

            if(!rgb)
                dengine_shader_set_vec3(&shader, "col", white);
            else
                dengine_shader_set_vec3(&shader, "col", rgb);


            quad.array.data = vertices;

            dengine_buffer_bind(GL_ARRAY_BUFFER, &quad.array);
            dengine_buffer_data(GL_ARRAY_BUFFER, &quad.array);
            dengine_buffer_bind(GL_ARRAY_BUFFER, NULL);

            dengine_texture_bind(GL_TEXTURE_2D, &fontmap);
            dengine_shader_use(&shader);

            //return previous src, dst alpha and blend
            int srcalpha, dstalpha;
            glGetIntegerv(GL_BLEND_SRC, &srcalpha); DENGINE_CHECKGL;
            glGetIntegerv(GL_BLEND_DST, &dstalpha); DENGINE_CHECKGL;
            int blnd;
            glGetIntegerv(GL_BLEND, &blnd); DENGINE_CHECKGL;

            if(!blnd)
                glEnable(GL_BLEND); DENGINE_CHECKGL;

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); DENGINE_CHECKGL;

            dengine_draw_primitive(&quad, &shader);

            glBlendFunc(srcalpha, dstalpha); DENGINE_CHECKGL;

            if(!blnd)
                glDisable(GL_BLEND); DENGINE_CHECKGL;

            dengine_texture_bind(GL_TEXTURE_2D, NULL);
            dengine_shader_use(NULL);
        }
    }
}
