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

#include "shader.h"
#include "texture.h"

/*! \struct Material
 *  Defines data in a Material
 */
typedef struct
{
    Shader shader_color; /*!< Color shader */
    Shader shader_shadow; /*!< 2D shadow shader */
    Shader shader_shadow3d; /*!< 3D shadow shader */

    uint32_t textures_targets[16]; /*!< Added Texture targets */
    Texture textures[16]; /*!< Added Textures */
    size_t textures_count; /*!< Added Texture count*/
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
 * \brief Set the color shader
 * \param shader Shader to use
 * \param material Material to use
 */
void dengine_material_set_shader_color(Shader* shader, Material* material);

/*!
 * \brief Set the 2D shadow shader
 * \param shader Shader to use
 * \param material Material to use
 */
void dengine_material_set_shader_shadow(Shader* shader, Material* material);

/*!
 * \brief Set the 3D shadow shader
 * \param shader Shader to use
 * \param material Material to use
 */
void dengine_material_set_shader_shadow3d(Shader* shader, Material* material);

/*!
 * \brief Add a texture to a material
 * \param target Can be GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP, ...
 * \param texture Texture to add
 * \param sampler Sampler to search in the color shader for assigning texture
 * \param material Material to use
 */
void dengine_material_add_texture(uint32_t target, Texture* texture, const char* sampler, Material* material);

/*!
 * \brief Use a material in a shader
 * \param material Material to use
 */
void dengine_material_use(Material* material);

#ifdef __cplusplus
}
#endif

#endif // MATERIAL_H
