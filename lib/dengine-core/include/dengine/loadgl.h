#ifndef LOADGL_H
#define LOADGL_H

#include "dengine_config.h" //GL, WIN
#include "checkgl.h"        //DENGINE_CHECKGL

#if defined (DENGINE_GL_GLAD)
    //USE GLAD
    #include <glad/glad.h>
#elif defined (DENGINE_GL_GLES2)
    //USE GLES 2.0
    #include <GLES2/gl2.h>
#elif defined (DENGINE_GL_GLES3)
    //USE GLES 3.0
    #include <GLES3/gl3.h>
#elif defined (DENGINE_GL_GLES31)
    //USE GLES 3.1
    #include <GLES3/gl31.h>
#elif defined(DENGINE_GL_GLES32)
    //USE GLES 3.2
    #include <GLES3/gl32.h>
#else
    #warning "Not using OpenGL. Define at least one OpenGL header / function loader to use OpenGL"
    #define DENGINE_GL_NONE
#endif

#endif // LOADGL_H
