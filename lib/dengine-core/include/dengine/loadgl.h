/*! \file loadgl.h
 *  Loads GL functions and check errors
 *
 *  Simply include this file to access GL functions. It includes the defined function loader in
 *  dengine_config.h
 */

#ifndef LOADGL_H
#define LOADGL_H

#include "dengine_config.h" //GL, WIN
#include "dengine/checkgl.h"        //DENGINE_CHECKGL;

#if defined (DENGINE_GL_GLAD)
    //USE GLAD
    #include <glad/glad.h>
#else
    #warning "Not using OpenGL. Define at least one OpenGL function loader to use OpenGL"
    #define DENGINE_GL_NONE
#endif

#endif // LOADGL_H
