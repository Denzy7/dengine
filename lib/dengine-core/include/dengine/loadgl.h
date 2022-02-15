#ifndef LOADGL_H
#define LOADGL_H

#include "dengine_config.h" //GL, WIN
#include "checkgl.h"        //DENGINE_CHECKGL;

#if defined (DENGINE_GL_GLAD)
    //USE GLAD
    #include <glad/glad.h>
#else
    #warning "Not using OpenGL. Define at least one OpenGL function loader to use OpenGL"
    #define DENGINE_GL_NONE
#endif

#endif // LOADGL_H
