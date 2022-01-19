#include "window.h"

#include "loadgl.h" //glViewport
#include "logging.h"//log

#include <stdio.h>  //printf

int isrunning = 0;
int glinit;
#if defined (DENGINE_WIN_GLFW)
GLFWwindow* glfw_current = NULL;
#endif // defined

int dengine_window_init()
{
    #if defined (DENGINE_WIN_GLFW)
    //Set callbacks
    glfwSetErrorCallback(dengine_window_glfw_callback_error);

    return glfwInit();
    #endif // DENGINE_WIN_GLFW
}

void dengine_window_terminate()
{
    #if defined (DENGINE_WIN_GLFW)
    glfwTerminate();
    #endif // defined
}

void dengine_window_request_GL(int gl_major, int gl_minor, int gl_core)
{
    #if defined (DENGINE_WIN_GLFW)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gl_major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gl_minor);

    if(gl_core)
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #endif // defined
}

void dengine_window_request_MSAA(int samples)
{
    #if defined (DENGINE_WIN_GLFW)
    glfwWindowHint(GLFW_SAMPLES, samples);
    #endif // defined
}

void dengine_window_get_window_width(int* width)
{
    #if defined (DENGINE_WIN_GLFW)
    glfwGetWindowSize(glfw_current, width, NULL);
    #endif // defined
}

void dengine_window_get_window_height(int* height)
{
    #if defined (DENGINE_WIN_GLFW)
    glfwGetWindowSize(glfw_current, NULL, height);
    #endif // defined
}

void dengine_window_set_viewport(int width, int height)
{
    #if !defined (DENGINE_GL_NONE)
    if(glinit)
        glViewport(0, 0, width, height);
    #endif // DENGINE_GL_NONE
}

void dengine_window_swapbuffers()
{
    #if defined(DENGINE_WIN_GLFW)
    glfwSwapBuffers(glfw_current);
    #endif // defined
}


int dengine_window_isrunning()
{
    return isrunning;
}

//GLFW Specific Calls
#if defined(DENGINE_WIN_GLFW)

int dengine_window_glfw_create(int width, int height, const char* title)
{
    glfw_current = glfwCreateWindow(width, height, title, NULL, NULL);

    if(glfw_current)
    {
        //set window callbacks

        //fbsize
        glfwSetFramebufferSizeCallback(glfw_current, dengine_window_glfw_callback_fbsize);

        //close
        glfwSetWindowCloseCallback(glfw_current, dengine_window_glfw_callback_windowclose);

        isrunning = 1;
    }

    return glfw_current ? 1 : 0;
}

void dengine_window_glfw_callback_error(int code, const char* description)
{
    dengineutils_logging_log("ERROR::GLFW::DESC::%s", description);
}

void dengine_window_glfw_set_monitor(GLFWmonitor* monitor, int xpos, int ypos, int refresh_rate)
{
    int w, h;
    glfwGetWindowSize(glfw_current, &w, &h);
    glfwSetWindowMonitor(glfw_current, monitor, xpos, ypos, w, h, refresh_rate);
}

void dengine_window_glfw_set_swapinterval(int interval)
{
    glfwSwapInterval(interval);
}

void dengine_window_glfw_callback_fbsize(GLFWwindow* window, int width, int height)
{
    dengine_window_set_viewport(width, height);
}

void dengine_window_glfw_callback_windowclose(GLFWwindow* window)
{
    isrunning = 0;
}

GLFWwindow* dengine_window_glfw_get_currentwindow()
{
    return glfw_current;
}

void dengine_window_glfw_pollevents()
{
    glfwPollEvents();
}

void dengine_window_glfw_context_makecurrent(GLFWwindow* window)
{
    glfwMakeContextCurrent(window);
}

int dengine_window_glfw_context_gladloadgl()
{
    #ifdef DENGINE_GL_GLAD
    glinit = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    return glinit;
    #else
    return 0;
    #endif // DENGINE_GL_GLAD
}
#endif // DENGINE_WIN_GLFW
