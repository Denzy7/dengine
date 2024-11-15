/*! \file material.h
 *  Execute material operations
 *
 *  A material has at least 16 textures which will be passed to shader when drawing
 *
 *  It also has a shader for color, 2d shadow and 3d shadow
 */

#ifndef MATERIAL_H
#define MATERIAL_H

#include <stddef.h> //size_t

#include "dengine/shader.h"
#include "dengine/texture.h"

#include "dengine_config.h"
typedef struct
{
    char sampler[DENGINE_MAX_MATERIAL_TEXTURE_SAMPLER_LENGTH];
    Texture texture;
    uint32_t target;
}MaterialTexture;

/* properties in the standard shaders */
typedef struct 
{
    float diffuse[3];
    float specular[3];
    float ambient[3];
    float alpha;
    float specular_power;
}MaterialProperties;

typedef uint32_t MaterialID;
/*! \struct Material
 *  Defines data in a Material
 */
typedef struct
{
    MaterialID id; /* tracked for scene batching */
    Shader shader_color; /*!< Color shader */
    Shader shader_shadow; /*!< 2D shadow shader */
    Shader shader_shadow3d; /*!< 3D shadow shader */

    MaterialTexture textures[DENGINE_MAX_MATERIAL_TEXTURES]; /*!< Added Textures */
    uint32_t textures_count; /*!< Added Texture count*/
    MaterialProperties properties; /*! Properties to upload to shader_color */

    Texture white; /*!< White texture this material uses */
    Texture normalmap; /*!< Empty normal map texture this material uses */

    int destroyed; /*!< HACK : Flag when material is destroyed to maintain compatibility */
}Material;

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Setup a material with some default values
 * \param material Material to setup
 */
void dengine_material_setup(Material* material);

/*!
 * \brief Set the color shader. This gets the textures expected to be used. Use before
 * setting texture
 * \param shader Shader to use
 * \param material Material to use
 */
void dengine_material_set_shader_color(const Shader* shader, Material* material);

/*!
 * \brief Set the 2D shadow shader
 * \param shader Shader to use
 * \param material Material to use
 */
void dengine_material_set_shader_shadow(const Shader* shader, Material* material);

/*!
 * \brief Set the 3D shadow shader
 * \param shader Shader to use
 * \param material Material to use
 */
void dengine_material_set_shader_shadow3d(const Shader* shader, Material* material);

/*!
 * \brief Set a texture in a material
 * \param texture Texture to add
 * \param sampler Sampler to search in the color shader for assigning texture
 * \param material Material to use
 */
int dengine_material_set_texture(const Texture* texture, const char* sampler, Material* material);

/*!
 * \brief Use a material in a shader. It sets material uniforms so beware it switches active shader
 * \param material Material to use. Careful when passing NULL as it unbinds everything!
 */
void dengine_material_use(const Material* material);

/*!
 * \brief Free resources being used by a material. Does NOT destroy the used textures.
 *  Only the sampler strings in the texture list and the white texture
 * \param material
 */
void dengine_material_destroy(Material* material);

/*!
 * \brief Get a texture stored in a material
 * \param sampler Sampler to search for in material
 * \param material Material to use
 * \return A texture object if found. NULL if not
 */
const Texture* dengine_material_get_texture(const char* sampler, Material* material);

#ifdef __cplusplus
}
#endif

#endif // MATERIAL_H
