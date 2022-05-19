#include "dengine/texture.h"
#include "dengine/loadgl.h"  //glGen,Bind,Tex...
#include "dengine-utils/logging.h" //log
#include "dengine/entrygl.h"

#include <string.h>
#include <stb_image.h> //stbi_load, stbi_error
#include <stb_image_write.h> //stbi_write_jpg

#include "dengine_config.h" //DENGINE_TEX_WHITESZ
#include "dengine-utils/debug.h"
#include "dengine-utils/filesys.h"
#include "dengine-utils/os.h"

static int texturecache = 0;

void _dengine_texture_autoload(Texture* texture);

void dengine_texture_gen(const size_t count, Texture* textures)
{
    DENGINE_DEBUG_ENTER;

    for(size_t i = 0; i < count; i++)
    {glGenTextures(1, &textures[i].texture_id); DENGINE_CHECKGL;}
}

void dengine_texture_bind(const uint32_t target, const Texture* texture)
{
    if(texture)
        glBindTexture(target, texture->texture_id);
    else
        glBindTexture(target, 0);
    DENGINE_CHECKGL;
}

void dengine_texture_data(const uint32_t target, const Texture* texture)
{
    DENGINE_DEBUG_ENTER;

    glTexImage2D(target, 0, texture->internal_format, texture->width, texture->height, 0,
                 texture->format, texture->type, texture->data);
    DENGINE_CHECKGL;
}

int dengine_texture_load_mem(const void* mem, const size_t size, const int flip, Texture* texture)
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

    // TODO : use DENGINE_PATHSEP. BUT MOST libs conform to using it
    const char* texfileondisk = strrchr(file, '/') + 1;
    char texcacheprtbf[2048];
    uint32_t cache_blk_sz = 0;

    if(dengineutils_filesys_isinit() && texturecache)
    {
        snprintf(texcacheprtbf, sizeof(texcacheprtbf),
                 "%s/%s/%s",
                 dengineutils_filesys_get_cachedir(),
                 DENGINE_TEXTURE_CACHE_DIR, DENGINE_VERSION);

        if(!dengineutils_os_direxist(texcacheprtbf))
            dengineutils_os_mkdir(texcacheprtbf);

        snprintf(texcacheprtbf, sizeof(texcacheprtbf),
                 "%s/%s/%s/%s%s",
                 dengineutils_filesys_get_cachedir(),
                 DENGINE_TEXTURE_CACHE_DIR, DENGINE_VERSION,
                 texfileondisk, DENGINE_TEXTURE_CACHE_EXT);

        FILE* ftex_cache_read = fopen(texcacheprtbf, "rb");
        if(ftex_cache_read)
        {
            //read blksz
            fread(&cache_blk_sz, sizeof(cache_blk_sz), 1, ftex_cache_read);

            //read w, h, and channels
            fread(&texture->width, sizeof(texture->width), 1, ftex_cache_read);
            fread(&texture->height, sizeof(texture->height), 1, ftex_cache_read);
            fread(&texture->channels, sizeof(texture->channels), 1, ftex_cache_read);

            texture->data = malloc(cache_blk_sz * texture->width * texture->height * texture->channels);
            //read data
            fread(texture->data,
                    cache_blk_sz * texture->width * texture->height * texture->channels,
                    1,
                    ftex_cache_read);

            dengineutils_logging_log("TODO::load tex bin %d-byte %dx%d %dch : %s",
                                      (int)cache_blk_sz,
                                      texture->width, texture->height,
                                      texture->channels,
                                      texfileondisk);
            //autodata
            if(texture->auto_dataonload)
                _dengine_texture_autoload(texture);

            return 1;
        }
    }

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
        cache_blk_sz = sizeof(uint8_t);
    }else if (texture->interface == DENGINE_TEXTURE_INTERFACE_16_BIT)
    {
        texture->data = stbi_load_from_file_16(fp,
                                              &texture->width, &texture->height,&texture->channels,
                                              0);
        cache_blk_sz = sizeof(uint16_t);
    }else if (texture->interface == DENGINE_TEXTURE_INTERFACE_FLOAT)
    {
        texture->data = stbi_loadf_from_file(fp,
                                              &texture->width, &texture->height,&texture->channels,
                                              0);
        cache_blk_sz = sizeof(float);
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

        if(dengineutils_filesys_isinit() && texturecache)
        {
            FILE* ftex_cache_write = fopen(texcacheprtbf, "wb");
            if(ftex_cache_write)
            {
                //write blksz
                fwrite(&cache_blk_sz, sizeof(cache_blk_sz), 1, ftex_cache_write);

                //write w, h, and channels
                fwrite(&texture->width, sizeof(texture->width), 1, ftex_cache_write);
                fwrite(&texture->height, sizeof(texture->height), 1, ftex_cache_write);
                fwrite(&texture->channels, sizeof(texture->channels), 1, ftex_cache_write);

                //write data
                fwrite(texture->data,
                        cache_blk_sz * texture->width * texture->height * texture->channels,
                        1,
                        ftex_cache_write);

                dengineutils_logging_log("TODO::save tex bin %d-byte %dx%d %dch : %s",
                                          (int)cache_blk_sz,
                                          texture->width, texture->height,
                                          texture->channels,
                                          texfileondisk);
                fclose(ftex_cache_write);
            }

        }

        if(texture->auto_dataonload)
            _dengine_texture_autoload(texture);
        return 1;
    }
}

void dengine_texture_set_params(uint32_t target, Texture* texture)
{
    DENGINE_DEBUG_ENTER;

    if(texture->filter_mag)
    {glTexParameteri(target, GL_TEXTURE_MAG_FILTER, texture->filter_mag); DENGINE_CHECKGL;}

    if(texture->filter_min)
    {glTexParameteri(target,GL_TEXTURE_MIN_FILTER,texture->filter_min); DENGINE_CHECKGL;}

    if(texture->wrap)
    {
        glTexParameteri(target, GL_TEXTURE_WRAP_S, texture->wrap); DENGINE_CHECKGL;
        glTexParameteri(target, GL_TEXTURE_WRAP_T, texture->wrap); DENGINE_CHECKGL;

        //GL_TEXTURE_WRAP_R_OES for GLES2?
        if(target == GL_TEXTURE_CUBE_MAP)
        {glTexParameteri(target, GL_TEXTURE_WRAP_R, texture->wrap); DENGINE_CHECKGL;}
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
    {glDeleteTextures(1, &textures[i].texture_id); DENGINE_CHECKGL;}
}

void dengine_texture_make_color(const int width, const int height, const float* color, const int channels, Texture* texture)
{
    DENGINE_DEBUG_ENTER;
    memset(texture, 0, sizeof (Texture));

    texture->format = channels == 3 ? GL_RGB : GL_RGBA;
    texture->internal_format = texture->format;

    void* data;
    if(dengine_texture_issupprorted(GL_TEXTURE_2D, GL_FLOAT, texture->internal_format, texture->format))
    {
        data = malloc(width * height * channels * sizeof(float));
        float* _data = data;
        for(size_t i = 0; i < width * height; i++)
        {
            for(int j = 0; j < channels; j++)
            {
                _data[i * channels + j] = color[j];
            }
        }
        texture->type = GL_FLOAT;
    }else
    {
        data =  malloc(width * height * channels);
        uint8_t* _data = data;
        for(size_t i = 0; i < width * height; i++)
        {
            for(int j = 0; j < channels; j++)
            {
                _data[i * channels + j] = color[j] * 255;
            }
        }
        texture->type = GL_UNSIGNED_BYTE;
    }

    texture->data = data;
    texture->filter_min = GL_NEAREST;
    texture->filter_mag = GL_NEAREST;
    texture->width = width;
    texture->height = height;

    const Texture* entrytex = dengine_entrygl_texture(GL_TEXTURE_2D);

    dengine_texture_gen(1, texture);
    dengine_texture_bind(GL_TEXTURE_2D, texture);
    dengine_texture_data(GL_TEXTURE_2D, texture);
    dengine_texture_set_params(GL_TEXTURE_2D, texture);
    dengine_texture_bind(GL_TEXTURE_2D, entrytex);

    free(data);
}

void dengine_texture_make_checkerboard(const int width, const int height,
                                         const int segments,
                                         unsigned char* foreground,
                                         unsigned char* background,
                                         int foreground_first,
                                         const int channels,
                                       Texture* texture)
{
    DENGINE_DEBUG_ENTER;

    memset(texture, 0, sizeof (Texture));
    texture->filter_min = GL_NEAREST;
    texture->filter_mag = GL_NEAREST;
    texture->width = width;
    texture->height = height;
    texture->format = channels == 3 ? GL_RGB : GL_RGBA;
    texture->internal_format = texture->format;
    texture->type = GL_UNSIGNED_BYTE;
    uint8_t* dat = malloc(width * height * channels);

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

    const Texture* entrytex = dengine_entrygl_texture(GL_TEXTURE_2D);

    texture->data = dat;
    dengine_texture_gen(1, texture);
    dengine_texture_bind(GL_TEXTURE_2D, texture);
    dengine_texture_data(GL_TEXTURE_2D, texture);
    dengine_texture_set_params(GL_TEXTURE_2D, texture);
    dengine_texture_bind(GL_TEXTURE_2D, entrytex);

    free(dat);
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

void dengine_texture_make_canreadback_color(const int width, const int height, Texture* texture)
{
    memset(texture, 0, sizeof(Texture));
    texture->width = width;
    texture->height = height;
    texture->type = GL_UNSIGNED_BYTE;
    texture->format = GL_RGBA;
    texture->internal_format = GL_RGBA;
    texture->filter_min = GL_LINEAR;
    texture->filter_mag = GL_LINEAR;
    uint8_t* data = calloc(width * height * 4, sizeof(uint8_t));
    texture->data = data;

    const Texture* entrytex = dengine_entrygl_texture(GL_TEXTURE_2D);

    dengine_texture_gen(1, texture);
    dengine_texture_bind(GL_TEXTURE_2D, texture);
    dengine_texture_data(GL_TEXTURE_2D, texture);
    dengine_texture_set_params(GL_TEXTURE_2D, texture);
    dengine_texture_bind(GL_TEXTURE_2D, entrytex);
}

int dengine_texture_issupprorted(uint32_t target, uint32_t type, uint32_t internal_format, uint32_t format)
{
    int supported = 1;
    uint32_t tex;
    int width = 8, height = 8;

    //clear errors
    glGetError();

    glGenTextures(1, &tex);
    glBindTexture(target, tex);

    //failed bind?
    if(glGetError())
        supported = 0;

    if(target == GL_TEXTURE_2D)
    {
        glTexImage2D(target, 0, internal_format, width, height, 0,
                     format, type, NULL);
    }else if(target == GL_TEXTURE_CUBE_MAP)
    {
        for(uint32_t i = 0; i < 6; i++)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internal_format, width, height, 0,
                         format, type, NULL);
        }
    }

    if(glGetError())
        supported = 0;

    glBindTexture(target, 0);
    glDeleteTextures(1, &tex);

    //clear errors
    glGetError();

    return supported;
}

const char* dengine_texture_target2str(uint32_t target)
{
    if(target == GL_TEXTURE_2D)
        return "TEXTURE_2D";
    else if(target == GL_TEXTURE_CUBE_MAP)
        return "TEXTURE_CUBE_MAP";
    else
        return "UNKNOWN";
}

void dengine_texture_set_texturecache(int state)
{
    texturecache = state;
}
