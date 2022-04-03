#include "dengine/texture.h"
#include "dengine/loadgl.h"  //glGen,Bind,Tex...
#include "dengine-utils/logging.h" //log

#include <string.h>
#include <stb_image.h> //stbi_load, stbi_error
#include <stb_image_write.h> //stbi_write_jpg

#include "dengine_config.h" //DENGINE_TEX_WHITESZ
#include "dengine-utils/debug.h"

void _dengine_texture_autoload(Texture* texture);

void dengine_texture_gen(size_t count, Texture* textures)
{
    DENGINE_DEBUG_ENTER;

    for(size_t i = 0; i < count; i++)
        glGenTextures(1, &textures[i].texture_id); DENGINE_CHECKGL;
}

void dengine_texture_bind(uint32_t target, Texture* texture)
{
    if(texture)
        glBindTexture(target, texture->texture_id);
    else
        glBindTexture(target, 0);
    DENGINE_CHECKGL;
}

void dengine_texture_data(uint32_t target, Texture* texture)
{
    DENGINE_DEBUG_ENTER;

    glTexImage2D(target, 0, texture->internal_format, texture->width, texture->height, 0,
                 texture->format, texture->type, texture->data);
    DENGINE_CHECKGL;
}

int dengine_texture_load_mem(void* mem, size_t size, int flip, Texture* texture)
{
    DENGINE_DEBUG_ENTER;

    stbi_set_flip_vertically_on_load(flip);

    if (texture->interface == DENGINE_TEXTURE_INTERFACE_8_BIT)
    {
        texture->data = stbi_load_from_memory(mem, size,
                                              &texture->width, &texture->height,&texture->channels,
                                              0);
    }else if (texture->interface == DENGINE_TEXTURE_INTERFACE_16_BIT)
    {
        texture->data = stbi_load_16_from_memory(mem, size,
                                              &texture->width, &texture->height,&texture->channels,
                                              0);
    }else if (texture->interface == DENGINE_TEXTURE_INTERFACE_FLOAT)
    {
        texture->data = stbi_loadf_from_memory(mem, size,
                                              &texture->width, &texture->height,&texture->channels,
                                              0);
    }else
    {
        dengineutils_logging_log("ERROR::TEXTURE::INVALID_INTERFACE");
        return 0;
    }

    if(!texture->data)
    {
        dengineutils_logging_log("ERROR::TEXTURE:STBI::%s", stbi_failure_reason());
        return 0;
    }else
    {
        if(texture->auto_dataonload)
            _dengine_texture_autoload(texture);
        return 1;
    }
}

int dengine_texture_load_file(const char* file, int flip, Texture* texture)
{
    DENGINE_DEBUG_ENTER;

    stbi_set_flip_vertically_on_load(flip);

    FILE* fp = fopen(file, "rb");
    if(!fp)
    {
        dengineutils_logging_log("ERROR::TEXTURE::CANNOT_FIND_FILE::%s", file);
        return 0;
    }

    if (texture->interface == DENGINE_TEXTURE_INTERFACE_8_BIT)
    {
        texture->data = stbi_load_from_file(fp,
                                          &texture->width, &texture->height,&texture->channels,
                                          0);
    }else if (texture->interface == DENGINE_TEXTURE_INTERFACE_16_BIT)
    {
        texture->data = stbi_load_from_file_16(fp,
                                              &texture->width, &texture->height,&texture->channels,
                                              0);
    }else if (texture->interface == DENGINE_TEXTURE_INTERFACE_FLOAT)
    {
        texture->data = stbi_loadf_from_file(fp,
                                              &texture->width, &texture->height,&texture->channels,
                                              0);
    }else
    {
        dengineutils_logging_log("ERROR::TEXTURE::INVALID_INTERFACE");
        return 0;
    }

    if(!texture->data)
    {
        dengineutils_logging_log("ERROR::TEXTURE::STBI::%s", stbi_failure_reason());
        return 0;
    }else
    {
        if(texture->auto_dataonload)
            _dengine_texture_autoload(texture);
        return 1;
    }
}

void dengine_texture_set_params(uint32_t target, Texture* texture)
{
    DENGINE_DEBUG_ENTER;

    if(texture->filter_mag)
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, texture->filter_mag); DENGINE_CHECKGL;

    if(texture->filter_min)
        glTexParameteri(target,GL_TEXTURE_MIN_FILTER,texture->filter_min); DENGINE_CHECKGL;

    if(texture->wrap)
    {
        glTexParameteri(target, GL_TEXTURE_WRAP_S, texture->wrap); DENGINE_CHECKGL;
        glTexParameteri(target, GL_TEXTURE_WRAP_T, texture->wrap); DENGINE_CHECKGL;

        //GL_TEXTURE_WRAP_R_OES for GLES2?
        if(target == GL_TEXTURE_CUBE_MAP)
            glTexParameteri(target, GL_TEXTURE_WRAP_R, texture->wrap); DENGINE_CHECKGL;
    }
}

void dengine_texture_free_data(Texture* texture)
{
    DENGINE_DEBUG_ENTER;

    if(texture->data)
        free(texture->data);
}

void dengine_texture_destroy(size_t count, Texture* textures)
{
    DENGINE_DEBUG_ENTER;

    for(size_t i = 0; i < count; i++)
        glDeleteTextures(1, &textures[i].texture_id); DENGINE_CHECKGL;
}

Texture* dengine_texture_new_white(const int width, const int height)
{
    DENGINE_DEBUG_ENTER;

    Texture* white = malloc(sizeof (Texture));
    memset(white, 0, sizeof (Texture));
    white->filter_min = GL_NEAREST;
    white->filter_mag = GL_NEAREST;
    white->height = width;
    white->width = height;
    white->internal_format = GL_RGB;
    white->format = GL_RGB;
    white->type = GL_UNSIGNED_BYTE;
    uint8_t* dat = malloc(sizeof (uint8_t) * width * height * 3 );
    memset(dat, 255, sizeof (uint8_t) * width * height * 3 );
    white->data = dat;
    dengine_texture_gen(1, white);
    dengine_texture_bind(GL_TEXTURE_2D, white);
    dengine_texture_data(GL_TEXTURE_2D, white);
    dengine_texture_set_params(GL_TEXTURE_2D, white);
    dengine_texture_bind(GL_TEXTURE_2D, NULL);
    return white;
}

Texture* dengine_texture_new_checkerboard(const int width, const int height,
                                         const int segments,
                                         unsigned char* foreground,
                                         unsigned char* background,
                                         int foreground_first,
                                         const int channels)
{
    DENGINE_DEBUG_ENTER;

    Texture* check = malloc(sizeof (Texture));
    memset(check, 0, sizeof (Texture));
    check->filter_min = GL_NEAREST;
    check->filter_mag = GL_NEAREST;
    check->width = width;
    check->height = height;
    check->format = channels == 3 ? GL_RGB : GL_RGBA;
    check->internal_format = check->format;
    check->type = GL_UNSIGNED_BYTE;
    uint8_t* dat = calloc(width * height * channels, sizeof (uint8_t));

    int swap = 0;
    int fill = foreground_first; /* set to 1 to fill fg first*/
    int box_sz = width / segments;
    uint8_t* col = NULL;
    for(int i = 0; i < width * height; i++)
    {
        if(i % (width * box_sz) == 0 && i != 0)
            swap = !swap;

        if(i % box_sz == 0 && i != 0)
            fill = !fill;

        if(fill)
        {
            if(swap)
                col = background;
            else
                col = foreground;
        }else
        {
            if(swap)
                col = foreground;
            else
                col = background;
        }

        for(int j = 0; j < channels; j++)
        {
            dat[i*channels + j] = col[j];
        }
    }

    check->data = dat;
    dengine_texture_gen(1, check);
    dengine_texture_bind(GL_TEXTURE_2D, check);
    dengine_texture_data(GL_TEXTURE_2D, check);
    dengine_texture_set_params(GL_TEXTURE_2D, check);
    dengine_texture_bind(GL_TEXTURE_2D, NULL);
    return check;
}

void dengine_texture_mipmap(uint32_t target, Texture* texture)
{
    DENGINE_DEBUG_ENTER;

    if(!glad_glGenerateMipmap)
    {
        dengineutils_logging_log("WARNING::TEXTURE::glGenerateMipmap not loaded. No mipmap generated");
        return;
    }

    dengine_texture_bind(GL_TEXTURE_2D, texture);
    glGenerateMipmap(target); DENGINE_CHECKGL;
    dengine_texture_bind(GL_TEXTURE_2D, NULL);
}

void _dengine_texture_autoload(Texture* texture)
{
    DENGINE_DEBUG_ENTER;

    texture->format=texture->channels==3?GL_RGB:GL_RGBA;
    texture->internal_format=texture->format;
    texture->type=GL_UNSIGNED_BYTE;
    texture->filter_min=GL_LINEAR;
    texture->filter_mag=GL_LINEAR;
    dengine_texture_gen(1,texture);
    dengine_texture_bind(GL_TEXTURE_2D,texture);
    dengine_texture_data(GL_TEXTURE_2D,texture);
    dengine_texture_set_params(GL_TEXTURE_2D,texture);
    dengine_texture_free_data(texture);
    dengine_texture_bind(GL_TEXTURE_2D,NULL);
}

int dengine_texture_writeout(const char* outfile, const int flip, Texture* texture)
{
    stbi_flip_vertically_on_write(flip);
    int comp = 3;
    if(texture->format == GL_RGBA)
        comp = 4;
    int write = stbi_write_jpg(outfile, texture->width, texture->height, comp, texture->data, 95);
    if(!write)
    {
        dengineutils_logging_log("ERROR::STBI_WRITE::FAILED!");
    }
    return write;
}

Texture* dengine_texture_new_canreadback_color(const int width, const int height)
{
    Texture* tex = malloc(sizeof(Texture));
    memset(tex, 0, sizeof(Texture));
    tex->width = width;
    tex->height = height;
    tex->type = GL_UNSIGNED_BYTE;
    tex->format = GL_RGBA;
    tex->internal_format = GL_RGBA;
    tex->filter_min = GL_LINEAR;
    tex->filter_mag = GL_LINEAR;
    uint8_t* data = calloc(width * height * 4, sizeof(uint8_t));
    tex->data = data;
    dengine_texture_gen(1, tex);
    dengine_texture_bind(GL_TEXTURE_2D, tex);
    dengine_texture_data(GL_TEXTURE_2D, tex);
    dengine_texture_set_params(GL_TEXTURE_2D, tex);
    dengine_texture_bind(GL_TEXTURE_2D, NULL);
    return tex;
}

Texture* dengine_texture_new_normalmap(const int width, const int height)
{
    Texture* tex = malloc(sizeof(Texture));
    memset(tex, 0, sizeof(Texture));
    tex->width = width;
    tex->height = height;
    tex->type = GL_UNSIGNED_BYTE;
    tex->format = GL_RGB;
    tex->internal_format = GL_RGB;
    tex->filter_min = GL_LINEAR;
    tex->filter_mag = GL_LINEAR;
    uint8_t* data = calloc(width * height * 3, sizeof(uint8_t));
    for(size_t i = 0; i < width * height; i++)
    {
        data[i * 3] = 255 / 2;
        data[i * 3 + 1] = 255 / 2;
        data[i * 3 + 2] = 255;
    }

    tex->data = data;
    dengine_texture_gen(1, tex);
    dengine_texture_bind(GL_TEXTURE_2D, tex);
    dengine_texture_data(GL_TEXTURE_2D, tex);
    dengine_texture_set_params(GL_TEXTURE_2D, tex);
    dengine_texture_bind(GL_TEXTURE_2D, NULL);
    return tex;
}
