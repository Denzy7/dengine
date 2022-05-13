#include "dengine/window.h"

#include "dengine-utils/logging.h"//log
#include "dengine/viewport.h" //set_view
#include "dengine/loadgl.h" //glad

#include <stdio.h>  //printf

static int _gl_max = 0, _gl_min = 0, _gl_core = 0 , _win_msaa = 0;
#ifdef DENGINE_WIN_EGL
EGLDisplay _egl_display;
EGLSurface _egl_surface;
EGLContext _egl_context;
#endif

int isrunning = 0;
int glinit = 0;
#if defined (DENGINE_WIN_GLFW)
GLFWwindow* glfw_current = NULL;
#endif // defined

Window _window;

int dengine_window_init()
{
    #if defined (DENGINE_WIN_GLFW)
    //Set callbacks
    glfwSetErrorCallback(dengine_window_glfw_callback_error);

    return glfwInit();

    #elif defined(DENGINE_WIN_EGL) && defined(DENGINE_ANDROID)
    /*
     * Basic EGL for Android. We can also use EGL
     * on Desktop too if we use XOpenDisplay and XCreateWindow
     * for X
     */
    _egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if(_egl_display == EGL_NO_DISPLAY)
    {
        dengineutils_logging_log("ERROR::WINDOW::CANNOT_OPEN_EGL_DISPLAY!");
        return 0;
    }

    EGLint major, minor;
    if(!eglInitialize(_egl_display, &major, &minor))
    {
        dengineutils_logging_log("ERROR::WINDOW::CANNOT_INIT_EGL!");
        return 0;
    }else
    {
        dengineutils_logging_log("INFO::WINDOW::EGL_INIT::v%d.%d", major, minor);
    }

    //EGL spec we want
    EGLint egl_spec_desired[] ={
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_RED_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_BLUE_SIZE, 8,
                EGL_ALPHA_SIZE, 8,
                EGL_DEPTH_SIZE, 24,
                EGL_SAMPLES, _win_msaa,
                EGL_NONE
    };
    //Get config count
    EGLint supported_count = 0;
    eglChooseConfig(_egl_display, egl_spec_desired, NULL, 0, &supported_count);

    if(supported_count == 0)
    {
        dengineutils_logging_log("ERROR::WINDOW::NO_SUITABLE_EGL_CONFIG_FOUND!");
        return 0;
    }

    dengineutils_logging_log("INFO::GOT_%d_CONFIGS.USING_0", supported_count);

    //Now get an actual config
    EGLConfig configs[supported_count];
    eglChooseConfig(_egl_display, egl_spec_desired, configs, supported_count, &supported_count);

    ANativeWindow* android_native_window = dengineutils_android_get_window();
    _egl_surface = eglCreateWindowSurface(_egl_display, configs[0], android_native_window , NULL);
    if(_egl_surface == EGL_NO_SURFACE)
    {
        dengineutils_logging_log("ERROR::WINDOW::NO_EGL_SURFACE!");
        return 0;
    }

    //The opengl context we want
    EGLint egl_context_attribs[] =
    {
        EGL_CONTEXT_MAJOR_VERSION, _gl_max,
        EGL_CONTEXT_MINOR_VERSION, _gl_min,
        EGL_NONE
    };

    _egl_context = eglCreateContext(_egl_display, configs[0], EGL_NO_CONTEXT, egl_context_attribs);
    if(_egl_context == EGL_NO_CONTEXT)
    {
        dengineutils_logging_log("ERROR::WINDOW::NO_EGL_CONTEXT!");
        return 0;
    }

    _window.display = _egl_display;
    _window.surface = _egl_surface;
    _window.context = _egl_context;

    if(!eglMakeCurrent(_egl_display, _egl_surface, _egl_surface, _egl_context))
    {
        dengineutils_logging_log("ERROR::WINDOW::CANNOT_MAKE_CONTEXT_CURRENT!");
        return 0;
    }

    return 1;

    #endif // DENGINE_WIN_GLFW
}

void dengine_window_terminate()
{
    #if defined (DENGINE_WIN_GLFW)
    glfwTerminate();
    #elif defined(DENGINE_WIN_EGL)
    if (_egl_display != EGL_NO_DISPLAY) {
        eglMakeCurrent(_egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (_egl_context != EGL_NO_CONTEXT) {
            eglDestroyContext(_egl_display, _egl_context);
        }
        if (_egl_surface != EGL_NO_SURFACE) {
            eglDestroySurface(_egl_display, _egl_surface);
        }
        eglTerminate(_egl_display);
    }
    #endif // defined
}

void dengine_window_request_GL(int gl_major, int gl_minor, int gl_core)
{
    _gl_max = gl_major;
    _gl_min = gl_minor;

}

void dengine_window_request_MSAA(int samples)
{
    _win_msaa = samples;
}

void dengine_window_request_defaultall()
{
#ifdef DENGINE_WIN_GLFW
    glfwDefaultWindowHints();
#endif
    _gl_max = 0;
    _gl_min = 0;
    _win_msaa = 0;
}

void dengine_window_get_window_width(int* width)
{
    #if defined (DENGINE_WIN_GLFW)
    glfwGetWindowSize(glfw_current, width, NULL);
    #elif defined (DENGINE_WIN_EGL)
    eglQuerySurface(_egl_display, _egl_surface, EGL_WIDTH, width);
    #endif // defined
}

void dengine_window_get_window_height(int* height)
{
    #if defined (DENGINE_WIN_GLFW)
    glfwGetWindowSize(glfw_current, NULL, height);
    #elif defined (DENGINE_WIN_EGL)
    eglQuerySurface(_egl_display, _egl_surface, EGL_HEIGHT, height);
    #endif // defined
}

void dengine_window_get_window_dim(int* width, int* height)
{
    if(height)
        dengine_window_get_window_height(height);

    if(width)
        dengine_window_get_window_width(width);
}

void dengine_window_swapbuffers()
{
    #if defined(DENGINE_WIN_GLFW)
    glfwSwapBuffers(glfw_current);
    #elif defined (DENGINE_WIN_EGL)
    eglSwapBuffers(_egl_display, _egl_surface);
    #endif // defined
}

int dengine_window_isrunning()
{
    #ifndef DENGINE_GL_NONE
    if(glinit)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    #endif // DENGINE_GL_NONE
    return isrunning;
}

int dengine_window_loadgl()
{
#if defined(DENGINE_WIN_GLFW)
    glinit = dengine_window_glfw_context_gladloadgl();
#elif defined(DENGINE_WIN_EGL)
    glinit = dengine_window_android_egl_context_gladloadgl();
#endif
    return glinit;
}

void dengine_window_makecurrent(Window* window)
{
#if defined(DENGINE_WIN_GLFW)
    dengine_window_glfw_context_makecurrent(window->window);
#elif defined(DENGINE_WIN_EGL)
    if(!eglMakeCurrent(window->display, window->surface, window->surface, window->context))
    {
        dengineutils_logging_log("ERROR::WINDOW::CANNOT_MAKE_CONTEXT_CURRENT!");
    }
#endif
}

void dengine_window_set_swapinterval(int interval)
{
#if defined(DENGINE_WIN_GLFW)
    dengine_window_glfw_set_swapinterval(interval);
#elif defined(DENGINE_WIN_EGL)
    if(!eglSwapInterval(_window.display, interval))
    {
        dengineutils_logging_log("ERROR::WINDOW::EGL cannot set swap interval");
    }
#endif
}

int dengine_window_create(int width, int height, const char* title, Window* window)
{
#if defined(DENGINE_WIN_GLFW)
    if (dengine_window_glfw_create(width, height, title)) {
        window->window = glfw_current;
        _window.window = glfw_current;
        return 1;
    } else
    {
        return 0;
    }
#else
    return 0;
#endif


}

//GLFW Specific Calls
#if defined(DENGINE_WIN_GLFW)

int dengine_window_glfw_create(int width, int height, const char* title)
{
    if(_gl_max)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, _gl_max);
    if(_gl_min)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, _gl_min);
    if(_gl_core)
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    if(_win_msaa)
        glfwWindowHint(GLFW_SAMPLES, _win_msaa);

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
    #if !defined (DENGINE_GL_NONE)
    if(glinit)
        dengine_viewport_set(0, 0, width, height);
    #endif // DENGINE_GL_NONE
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
    return gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    #else
    return 0;
    #endif // DENGINE_GL_GLAD
}
#endif // DENGINE_WIN_GLFW

#ifdef DENGINE_ANDROID
int dengine_window_android_egl_context_gladloadgl()
{
    return gladLoadGLES2Loader((GLADloadproc)eglGetProcAddress);
}
#endif
