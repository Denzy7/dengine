#include "dengine-gui/gui.h"
#include "dengine-gui/embfonts.h"

#include "dengine/buffer.h"
#include "dengine/texture.h"
#include "dengine/loadgl.h"
#include "dengine/primitive.h"
#include "dengine/shader.h"
#include "dengine/draw.h"
#include "dengine/input.h"
#include "dengine/entrygl.h"
#include "dengine/viewport.h"

#include "dengine-utils/logging.h"
#include "dengine-utils/debug.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h> //stbtt

#include "dengine_config.h"

#ifdef DENGINE_HAS_FONTCONFIG
#include <fontconfig/fontconfig.h>
char* fcfilestrbuf = NULL;
const int fcfilestrbufsz = 2048;
#endif


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
int btnrepeatable = 0;
int paneldiscard = 0;
float discardthreshold = 0.1f;

//the standard gui quad
Primitive quad;

Shader gui_text;
Shader gui_panel;
Shader gui_discard;

int initgui = 0;
#define PANEL_ALPHA 0.4f
Texture dftpaneltex;

const char* _denginegui_get_defaultfont()
{
    #if defined (DENGINE_FONT_DEFAULT)
    return DENGINE_FONT_DEFAULT;
    #elif defined (DENGINE_HAS_FONTCONFIG)
    FcPattern* pat, *match;
    FcResult result;
    FcChar8* file = NULL;

    pat = FcNameParse((FcChar8*) DENGINE_FONTCONFIG_MATCHPATTERN);
    FcConfigSubstitute (0, pat, FcMatchPattern);
    FcDefaultSubstitute (pat);
    match = FcFontMatch (0, pat, &result);
    if (match)
    {
        if(FcPatternGetString(match, FC_FILE, 0, &file) == FcResultMatch)
        {
            snprintf(fcfilestrbuf, fcfilestrbufsz, "%s", file);
        }
    }
    FcPatternDestroy (pat);
    FcPatternDestroy (match);
    return fcfilestrbuf;
    #else
    return NULL;
    #endif
}

int denginegui_init()
{
    DENGINE_DEBUG_ENTER;

    static float vertices[16];

    static uint16_t indices[]=
    {
        0, 3, 2, 2, 1, 0
    };

    quad.draw_mode = GL_TRIANGLES;
    quad.draw_type = GL_UNSIGNED_SHORT;

    quad.array.data = vertices;
    quad.array.size = sizeof(vertices);
    quad.array.usage = GL_STREAM_DRAW;

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

    int setup = 
        dengine_shader_make_standard(DENGINE_SHADER_GUI_TEXT, &gui_text) &&
        dengine_shader_make_standard(DENGINE_SHADER_GUI_PANEL, &gui_panel) &&
        dengine_shader_make_standard(DENGINE_SHADER_GUI_DISCARD, &gui_discard);
        

    #ifdef DENGINE_HAS_FONTCONFIG
    fcfilestrbuf = calloc(fcfilestrbufsz, sizeof(char));
    #endif

    if(setup)
    {
        dengine_shader_set_int(&gui_text, "tex", 0);
        dengine_shader_set_int(&gui_discard, "tex", 0);
        dengine_shader_set_int(&gui_panel, "tex", 0);

        /* all use the same vertex shader, so vertex
         * attribs should be the same
         */
        dengine_primitive_setup(&quad, &gui_text);
        static const float dftpaneltex_col[3] = {0.1, 0.2, 0.3};
        dengine_texture_make_color(8, 8, dftpaneltex_col, 3, &dftpaneltex);
        initgui = 1;
        return 1;
    }else
    {
        printf("cannot init quad shader\n");
        return 0;
    }
}

void denginegui_terminate()
{
    DENGINE_DEBUG_ENTER;

    dengine_texture_destroy(1, &fontmap);
    dengine_shader_destroy(&gui_text);
    dengine_shader_destroy(&gui_panel);
    dengine_shader_destroy(&gui_discard);
    dengine_primitive_destroy(&quad);

#ifdef DENGINE_HAS_FONTCONFIG
    free(fcfilestrbuf);
#endif
}

int denginegui_set_font(const void* ttf, const float fontsize, const uint32_t bitmap_size)
{
    DENGINE_DEBUG_ENTER;

    const void* mem = ttf;
    void* sysfontmem = NULL;
    if (!ttf) {
        const char* file = _denginegui_get_defaultfont();
        FILE* f = fopen(file, "rb");
        if (!f) {
            dengineutils_logging_log("WARNING::cannot read default font %s", file);
            mem = OpenSans_Light_ttf;
            dengineutils_logging_log("WARNING::Load embfont OpenSans_Light");
        }else
        {
            fseek(f, 0, SEEK_END);
            size_t sz = ftell(f);
            rewind(f);
            sysfontmem = malloc(sz);
            fread(sysfontmem, sz, 1, f);
            mem = sysfontmem;
            fclose(f);
        }
    }

    if(!stbtt_InitFont(&info, mem, 0))
    {
        dengineutils_logging_log("ERROR::failed to init font\n");
        return 0;
    }

    const size_t sz_bmp = bitmap_size * bitmap_size;
    unsigned char* baked_bmp = malloc(sizeof(unsigned char) * sz_bmp);
    memset(baked_bmp, 0, sizeof(unsigned char) * sz_bmp);

    stbtt_pack_context ctx;
    if(!stbtt_PackBegin(&ctx, baked_bmp, bitmap_size, bitmap_size, 0, 1, NULL))
    {
        dengineutils_logging_log("ERROR::failed to pack chars\n");
        //return 0;
    }

    if(!stbtt_PackFontRange(&ctx, mem, 0, fontsize, 32, 96, packedchar_data))
    {
        dengineutils_logging_log("ERROR::failed to pack chars range\n");
        //return 0;
    }
    stbtt_PackEnd(&ctx);

    if(!initfont)
        memset(&fontmap, 0, sizeof(Texture));

    //Nasty hack to use rgb to dodge various GL/ES quirks
    unsigned char* baked_bmp_rgb = malloc(sizeof (unsigned char) * sz_bmp * 3);
    memset(baked_bmp_rgb, 0, sizeof(unsigned char) * sz_bmp * 3);
    for (size_t i = 0; i < sz_bmp; i++) {
        baked_bmp_rgb[i * 3] = baked_bmp[i];
    }

    fontmap.format = GL_RGB;
    fontmap.internal_format = GL_RGB;
    fontmap.type = GL_UNSIGNED_BYTE;

    fontmap.data = baked_bmp_rgb;
    fontmap.filter_min = GL_LINEAR;

    fontmap.width = bitmap_size;
    fontmap.height = bitmap_size;

    if(!initfont)
    {
        dengine_texture_gen(1, &fontmap);
        initfont = 1;
    }

    _bmp_sz = bitmap_size;

    Texture entry_tex;
    dengine_entrygl_texture(GL_TEXTURE_2D, &entry_tex);

    dengine_texture_bind(GL_TEXTURE_2D, &fontmap);
    dengine_texture_data(GL_TEXTURE_2D, &fontmap);
    dengine_texture_set_params(GL_TEXTURE_2D, &fontmap);
    dengine_texture_bind(GL_TEXTURE_2D, &entry_tex);

    _fontsz = fontsize;

    if(sysfontmem)
        free(sysfontmem);

    free(baked_bmp);
    free(baked_bmp_rgb);

    return 1;
}

void denginegui_set_button_repeatable(int state)
{
    btnrepeatable = state;
}

void _denginegui_projectquad()
{
    mat4 projection;
    int w, h;
    dengine_viewport_get(NULL, NULL, &w, &h);
    glm_ortho(0.0, w, 0.0, h, -1.0f, 1.0f, projection);
    dengine_shader_current_set_mat4("projection", projection[0]);
}

int srcalpha, dstalpha, blnd, depthmask;
/* setup up blending */
void _denginegui_beginquad()
{
    //return previous src, dst alpha and blend

    glGetIntegerv(GL_BLEND_SRC_ALPHA, &srcalpha); DENGINE_CHECKGL;
    glGetIntegerv(GL_BLEND_DST_ALPHA, &dstalpha); DENGINE_CHECKGL;

    glGetIntegerv(GL_BLEND, &blnd); DENGINE_CHECKGL;

    if(!blnd)
    {glEnable(GL_BLEND); DENGINE_CHECKGL;}

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); DENGINE_CHECKGL;

    glGetIntegerv(GL_DEPTH_WRITEMASK, &depthmask); DENGINE_CHECKGL;
    if (depthmask)
        glDepthMask(GL_FALSE);
}

void _denginegui_endquad()
{
    glBlendFunc(srcalpha, dstalpha); DENGINE_CHECKGL;

    if(!blnd)
    {glDisable(GL_BLEND); DENGINE_CHECKGL;}

    if (depthmask)
    {glDepthMask(GL_TRUE); DENGINE_CHECKGL;}
}

void denginegui_text(float x, float y, const char* text, float* rgba)
{
    DENGINE_DEBUG_ENTER;

    if(!initgui || !initfont)
        return;

    // get entry stuff
    Texture entry_tex;
    int entry_activetex = 0;
    if(dengine_entrygl_get_enabled())
    {
        dengine_entrygl_texture(GL_TEXTURE_2D, &entry_tex);
        dengine_entrygl_texture_active(&entry_activetex);
    }

    // set texture
    glActiveTexture(GL_TEXTURE0);
    dengine_texture_bind(GL_TEXTURE_2D, &fontmap);
    DENGINE_CHECKGL;

    // set color
    float white[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    dengine_shader_use(&gui_text);
    if(!rgba)
        dengine_shader_current_set_vec4("col", white);
    else
        dengine_shader_current_set_vec4("col", rgba);

    _denginegui_projectquad();
    _denginegui_beginquad();
    dengine_draw_sequence_start(&quad, &gui_text);
    //bind array buffer since we'll be changing it after getting packed quad
    dengine_buffer_bind(GL_ARRAY_BUFFER, &quad.array);

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

            quad.array.data = vertices;
            // draw our stuff, update buffer
            dengine_buffer_data(GL_ARRAY_BUFFER, &quad.array);
            /* for text, we need to sequence draw since
             * everything is same quad, different data
             */
            dengine_draw_sequence_draw();
        }
    }
    dengine_draw_sequence_end();
    _denginegui_endquad();

    if(dengine_entrygl_get_enabled())
    {
        // return entry stuff
        dengine_texture_bind(GL_TEXTURE_2D, &entry_tex);
        glActiveTexture(entry_activetex);
        DENGINE_CHECKGL;
    }
}

void denginegui_panel(float x, float y, float width, float height, Texture* texture, float* uv, float* rgba)
{
    DENGINE_DEBUG_ENTER;

    if(!initgui)
        return;

    // get entry stuff
    Texture entry_tex;
    int entry_activetex = 0;
    if(dengine_entrygl_get_enabled())
    {
        dengine_entrygl_texture(GL_TEXTURE_2D, &entry_tex);
        dengine_entrygl_texture_active(&entry_activetex);
    }

    //set texture
    glActiveTexture(GL_TEXTURE0);
    DENGINE_CHECKGL;
    if(texture)
        dengine_texture_bind(GL_TEXTURE_2D, texture);
    else
        dengine_texture_bind(GL_TEXTURE_2D, &dftpaneltex);

    //set col
    float white[4] = {0.0f, 0.0f, 0.0f, PANEL_ALPHA};
    Shader* shader = &gui_panel;
    if(paneldiscard)
        shader = &gui_discard;

    dengine_shader_use(shader);
    if(!rgba)
        dengine_shader_current_set_vec4("col", white);
    else
        dengine_shader_current_set_vec4("col", rgba);

    if(paneldiscard)
        dengine_shader_set_float(shader, "threshold", discardthreshold);

    _denginegui_projectquad();
    //start seq
    _denginegui_beginquad();
    dengine_draw_sequence_start(&quad, shader);
    //bind abuf since will change it below
    dengine_buffer_bind(GL_ARRAY_BUFFER, &quad.array);

    float vertices[]=
    {
                      // Vert,   TexCoord
                x,          y, 0.0f, 0.0f,
                x, y + height, 0.0f, 1.0f,
        x + width, y + height, 1.0f, 1.0f,
        x + width,          y, 1.0f, 0.0f,
    };
    quad.array.data = vertices;
    dengine_buffer_data(GL_ARRAY_BUFFER, &quad.array);
    dengine_draw_sequence_draw();
    dengine_draw_sequence_end();
    _denginegui_endquad();

    // return entry stuff
    if(dengine_entrygl_get_enabled())
    {
        glActiveTexture(entry_activetex);
        dengine_texture_bind(GL_TEXTURE_2D, &entry_tex);
        DENGINE_CHECKGL;
    }
}

int denginegui_button(float x,float y, float width, float height, const char* text, float* rgba)
{
    DENGINE_DEBUG_ENTER;

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

        down = btnrepeatable ? dengine_input_get_mousebtn(0) : dengine_input_get_mousebtn_once(0);
    }else
    {
        denginegui_panel(x, y, width, height, NULL, NULL, rgba);
    }

    int scissor = glIsEnabled(GL_SCISSOR_TEST);
    DENGINE_CHECKGL;
    if(!scissor)
    {
        glEnable(GL_SCISSOR_TEST);
        DENGINE_CHECKGL;
    }
    glScissor(x, y, width, height);
    DENGINE_CHECKGL;

    float txtoffst = (height / 2) - (_fontsz / 4);
    denginegui_text(x + (txtoffst / 2), y + txtoffst, text, NULL);

    if(!scissor)
    {
        glDisable(GL_SCISSOR_TEST);
        DENGINE_CHECKGL;
    }

    return down;
}

float denginegui_get_fontsz()
{
    return _fontsz;
}

void denginegui_set_panel_discard(int state)
{
    paneldiscard = state;
}

void denginegui_set_panel_discard_threshold(float threshold)
{
    discardthreshold = threshold;
}
