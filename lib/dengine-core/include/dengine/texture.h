/*! \file texture.h
 *  Execute texture operations
 */

#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdint.h> //uint32
#include <stddef.h> //size_t

/*! \enum TextureInterface
 *  Defines how a texture has been aligned in memory when loading
 *
 *  Interfaces can come in various forms like 8 bits/channel where
 *  if a texture is RGB, each bit (R, G and B) has 8 bits (unsigned char) each with a color 0-255
 *
 *  You must specify the correct format otherwise stb_image (image loader) will fail. A general rule
 *  of thumb is that normal images are 8-bit. HDR textures are 16-bit. Float textures can be depth
 *  maps, grayscale images which are rarely used
 */
typedef enum
{
    DENGINE_TEXTURE_INTERFACE_8_BIT = 1,
    DENGINE_TEXTURE_INTERFACE_16_BIT,
    DENGINE_TEXTURE_INTERFACE_FLOAT
}TextureInterface;

/*!
 * \struct Texture
 * Define a texture object
 *
 * A texture contains an array of colors aligned to a specific format and
 * of a specific width and height
 *
 * A texture can come in various flavors but 2D are the most common. A cubemap
 * is a 2D texture that has been applied to all 6 faces of a cube hence its name
 */
typedef struct
{
    uint32_t texture_id; /*!< Texture id. DON'T change this */
    int width; /*!< Width of the texture */
    int height; /*!< Height of the texture */
    int channels; /*!< Color channels in the texture */
    void* data; /*!< Pointer to where texture data is located */
    TextureInterface interface; /*!< How data is packed in the pointer */
    uint32_t format ; /*!< How data is going to be aligned for each channel */
    uint32_t internal_format; /*!< More or less the same as format with some additional special formats or compression*/
    uint32_t type; /*!< How each bit of color is sized; UNSIGNED_BYTE, FLOAT, ... */

    //params to glTexParameteri
    uint32_t filter_mag; /*!< Magnification filer; LINEAR, NEAREST,...  */
    uint32_t filter_min; /*!< Minification filter; Like magnification. Always set this */
    uint32_t wrap; /*!< Wrapping; CLAMP, REPEAT, ... */

    char* name; /*!< Unused. Name of texture */
    int auto_dataonload; /*!< Automatically fill texture once its loaded. Only for 2D texture */
}Texture;

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Generate some textures
 * \param count Number of textures
 * \param textures Pointer to texture array
 */
void dengine_texture_gen(const size_t count, Texture* textures);

/*!
 * \brief Bind a texture to target. NULL unbinds the texture bound at target
 * \param target Target to bind
 * \param texture Texture to bind
 */
void dengine_texture_bind(const uint32_t target, const Texture* texture);

/*!
 * \brief Update the data in the currently bound texture at target
 * \param target Target to use
 * \param texture Texture to use
 */
void dengine_texture_data(const uint32_t target, const Texture* texture);

/*!
 * \brief Attempt to load from memory
 * \param mem Memory location
 * \param size Size of memory
 * \param flip Flip y axis on load
 * \param texture Texture to use
 * \return 0 on failure
 */
int dengine_texture_load_mem(const void* mem, const size_t size, const int flip, Texture* texture);

/*!
 * \brief Attempt to load from filesystem
 * \param file Filename to load
 * \param flip Flip y axis on load
 * \param texture Texture to use
 * \return
 */
int dengine_texture_load_file(const char* file, int flip, Texture* texture);

/*!
 * \brief Set texture parameters of the currently bound texture at target
 * \param target Target to use
 * \param texture Texture to use
 */
void dengine_texture_set_params(uint32_t target, Texture* texture);

/*!
 * \brief Free data that was loaded with texture_load_* or texture_make_canreadback_* as its no longer needed
 * \param texture Texture to use
 */
void dengine_texture_free_data(Texture* texture);

/*!
 * \brief Destroy generated textures
 * \param count Number of textures
 * \param textures Pointer to textures
 */
void dengine_texture_destroy(size_t count, Texture* textures);

/*!
 * \brief Make a texture of specified color
 * \param width width
 * \param height height
 * \param color color (0.0-1.0 per channel) in float array
 * \param channels channels in color (preferably 3 for rgb or 4 for rgba)
 * \param texture Texture to make
 */
void dengine_texture_make_color(const int width, const int height, const float* color, const int channels, Texture* texture);

/*!
 * \brief Make a checkerboard pattern in texture
 * \param width width
 * \param height height
 * \param segments segments to split width and height
 * \param foreground foreground color (0.0-1.0 per channel) in float array
 * \param background background color (0.0-1.0 per channel) in float array
 * \param foreground_first 1 = foreground first, 0 = background first
 * \param channels color channels in both foreground and background (preferably 3 or 4)
 * \param texture Texture to make
 */
void dengine_texture_make_checkerboard(const int width, const int height,
                                         const int segments,
                                         const float* foreground,
                                         const float* background,
                                         const int foreground_first,
                                         const int channels,
                                       Texture* texture);

/*!
 * \brief Make a Texture that can readback framebuffer color attachments.
 * Remember to free data after reading back.
 * \param width width of texture
 * \param height height of texture
 */
void dengine_texture_make_canreadback_color(const int width, const int height, Texture* texture);

/*!
 * \brief Bind texture then generate mipmap then unbind
 * \param target Target to use
 * \param texture Texture to use
 */
void dengine_texture_mipmap(uint32_t target, Texture* texture);

/*!
 * \brief Write out a texture to a file (JPG compression, 95% quality)
 * \param outfile File to use
 * \param flip Flip vertically on write
 * \param texture Texture to use
 * \return 0 if failed, else success
 */
int dengine_texture_writeout(const char* outfile, const int flip, Texture* texture);

/*!
 * \brief Check if a texture format is supported by GPU
 * \param target target to use
 * \param type type of texture
 * \param internal_format internal_format to check
 * \param format format to check
 * \return Non-zero if supported
 */
int dengine_texture_issupprorted(uint32_t target, uint32_t type, uint32_t internal_format, uint32_t format);

/*!
 * \brief Return a human friendly string of a texture target
 * \param target target to use
 * \return Human friendly string of a texture target
 */
const char* dengine_texture_target2str(uint32_t target);

/*!
 * \brief Set texture cache state. 0 = disable, 1 = enable
 * \param state State
 */
void dengine_texture_set_texturecache(int state);


#ifdef __cplusplus
}
#endif

#endif // TEXTURE_H
