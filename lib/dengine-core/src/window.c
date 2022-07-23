#include "dengine/window.h"

#include "dengine/viewport.h" //set_view
#include "dengine/loadgl.h" //glad
#include "dengine/input.h" //setwindow
#include "dengine-utils/logging.h"//log
#include "dengine-utils/dynlib.h" //getsym
#include "dengine-utils/thread.h"
#include "dengine-utils/macros.h" //ARY_SZ

#include <stdio.h>  //printf
#include <stdlib.h> //malloc
#include <string.h> //memcpy
#include <ctype.h> //toupper

//WINDOW CREATION INCL.
#ifdef DENGINE_WIN_X11
#include <X11/Xlib.h> //Window
#include <X11/XKBlib.h>
#elif defined (DENGINE_WIN32)
#include <windows.h> //HWND
#include <windowsx.h> //GET_X_LPARAM
#elif defined (DENGINE_ANDROID)
#include <dengine-utils/platform/android.h> //ANativeWindow
#else
#error "No suitable Window creation framework found"
#endif

//CONTEXT CREATION INCL.
#ifdef DENGINE_CONTEXT_EGL
#include <EGL/egl.h>
#elif defined (DENGINE_CONTEXT_GLX)
#include <GL/glx.h>
#elif defined (DENGINE_CONTEXT_WGL)
#include <GL/wgl.h>
#endif

//unfreeze gtk
#ifdef DENGINE_HAS_GTK3
#include <gtk/gtk.h>
#endif

#ifdef DENGINE_ANDROID
#define GL "libGLESv3.so"
#elif defined(DENGINE_LINUX)
#define GL "libGL.so.1"
#elif defined(DENGINE_WIN32)
#define GL "opengl32.dll"
#endif

#ifdef DENGINE_WIN32
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

void _dengine_window_processkey(WindowInput* input, char key, int isrelease);
void* _dengine_window_pollinf(void* arg);

#ifdef DENGINE_CONTEXT_EGL
int _dengine_window_egl_createctx(EGLDisplay dpy, EGLSurface* egl_sfc, EGLContext* egl_ctx, EGLContext share, EGLNativeWindowType win);
#endif
struct DengineWindow
{
    ContextType type;
    int running;
    int width, height;
#ifdef DENGINE_WIN_X11
    Window x_win;
    XSetWindowAttributes x_swa;
    XEvent ev;
#endif
#ifdef DENGINE_CONTEXT_GLX
    GLXContext glx_ctx;
#endif
#ifdef DENGINE_WIN32
    HWND win32_hwnd;
    HWND win32_after;
    HGLRC win32_ctx;
    HGLRC win32_ctx_shr;
    MSG win32_msg;
#endif
#ifdef DENGINE_CONTEXT_EGL
    EGLDisplay egl_dpy;
    EGLSurface egl_sfc;
    EGLContext egl_ctx;
#endif
#ifdef DENGINE_ANDROID
    ANativeWindow* and_win;
#endif
    int gl_load;
    DynLib gl_lib;
    WindowInput input;
};
#define DFT_GL_MAX 2
#define DFT_GL_MIN 0
#define DFT_WIN_MSAA 0
#define DFT_WIN_DEPTH 24
static int _gl_max = DFT_GL_MAX, _gl_min = DFT_GL_MIN; /*_gl_core = 0 , */
static int _win_msaa = DFT_WIN_MSAA, _win_depth = DFT_WIN_DEPTH;


#ifdef DENGINE_WIN_X11
Display* x_dpy;
Atom wm_delete;
#endif
#ifdef DENGINE_CONTEXT_EGL
EGLDisplay egl_dpy;
#endif
#ifdef DENGINE_CONTEXT_GLX
PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB = NULL;
PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT = NULL;
#endif
#ifdef DENGINE_CONTEXT_WGL
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = NULL;
PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
#endif
#ifdef DENGINE_WIN32
WNDCLASSW wc = { };
static const wchar_t* wc_class = L"Dengine_Win32";
#endif

DengineWindow* current = NULL;
DynLib gl = NULL;

int dengine_window_init()
{
    int init = 0;
#ifdef DENGINE_WIN_X11
    XInitThreads();
    x_dpy = XOpenDisplay(NULL);
    if(x_dpy)
    {
        wm_delete = XInternAtom(x_dpy, "WM_DELETE_WINDOW", False);
        //disable annoying repeat
        int setdetect = XkbSetDetectableAutoRepeat(x_dpy, True, NULL);
        if(!setdetect)
        {
            dengineutils_logging_log("WARNING::Could not disable autorepeat with "
                                     "XkbSetDetectableAutoRepeat. "
                                     "This will cause issues with input_key_get_once");

        }
#ifdef DENGINE_CONTEXT_EGL
        egl_dpy = eglGetDisplay((EGLNativeDisplayType)x_dpy);
#endif
#ifdef DENGINE_CONTEXT_GLX
        //LOOK FOR GLX EXTENSIONS
        glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC) glXGetProcAddress((GLubyte*) "glXSwapIntervalEXT");
        glXCreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC) glXGetProcAddress((GLubyte*)"glXCreateContextAttribsARB");
#endif
        init = 1;
    }else
    {
        dengineutils_logging_log("ERROR::WINDOW::CANNOT_OPEN_X_DISPLAY!");
        init = 0;
    }
#elif defined(DENGINE_ANDROID)
    egl_dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if(egl_dpy == EGL_NO_DISPLAY)
    {
        dengineutils_logging_log("ERROR::WINDOW::CANNOT_OPEN_EGL_DISPLAY!");
        init = 0;
    }else
    {
        init = 1;
    }
#elif defined(DENGINE_WIN32)
    wc.hInstance = GetModuleHandle(NULL);;
    wc.lpszClassName = wc_class;
    wc.lpfnWndProc = WndProc;
    wc.style = CS_OWNDC;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    if(RegisterClassW(&wc))
    {
        init = 1;
    }else
    {
        dengineutils_logging_log("ERROR::WINDOW::Cannot register Win32 Window Class!");
        init = 0;
    }
#endif

    //at this point nothing works if not init display
    if(!init)
        return 0;

#ifdef DENGINE_CONTEXT_EGL
    EGLint major, minor;
    if(!eglInitialize(egl_dpy, &major, &minor))
    {
        dengineutils_logging_log("ERROR::WINDOW::CANNOT_INIT_EGL!");
    }else
    {
        dengineutils_logging_log("INFO::WINDOW::EGL_INIT::v%d.%d", major, minor);
    }
#endif

#ifdef DENGINE_ANDROID
    /*
     * directly create current (android only supports 1 window and display afaik)
     * and make it current
     */
    DengineWindow* _andr = dengine_window_create(0, 0, NULL, NULL);
    dengine_window_makecurrent(_andr);
    dengine_window_loadgl(_andr);
#endif
    return init;
}

void dengine_window_terminate()
{
#ifdef DENGINE_CONTEXT_GLX
    glXMakeCurrent(x_dpy, None, NULL);
#endif

#ifdef DENGINE_CONTEXT_EGL
#ifndef DENGINE_ANDROID
    //crash on android. but leaks on others
    eglMakeCurrent(egl_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglTerminate(egl_dpy);
#endif
#endif

#ifdef DENGINE_WIN_X11
    XCloseDisplay(x_dpy);
#endif
#ifdef DENGINE_WIN32
    UnregisterClassW(wc_class, wc.hInstance);
#endif

}

void _dengine_window_processkey(WindowInput* input, char key, int isrelease)
{
    if(key >= 33 && key <= 126)
    {
        if(isrelease)
        {
            for (int i = 0; i < DENGINE_WINDOW_ALPNUM; i++) {
                if(input->alpnum[i].key == key)
                {
                    input->alpnum[i].state = 0;
                    input->alpnum[i].key = 0;
                }
            }
//            for (int i = 0; i < DENGINE_WINDOW_ALPNUM; i++)
//            {
//                printf("[%c] ", input->alpnum[i].key);
//            }
//            printf("\n");
        }else
        {
            for (int i = 0; i < DENGINE_WINDOW_ALPNUM; i++) {
                //dont dupe;
                if(input->alpnum[i].key == key)
                    break;

                if(input->alpnum[i].key != key &&
                        input->alpnum[i].key == 0 &&
                        input->alpnum[i].state != -1)
                {
                    input->alpnum[i].key = key;
                    break;
                }
            }
//            for (int i = 0; i < DENGINE_WINDOW_ALPNUM; i++)
//            {
//                printf("[%c] ", input->alpnum[i].key);
//            }
//            printf("\n");
        }

    }
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
    _gl_max = DFT_GL_MAX;
    _gl_min = DFT_GL_MIN;
    _win_msaa = DFT_WIN_MSAA;
    _win_depth = DFT_WIN_DEPTH;
}

DengineWindow* dengine_window_create(int width, int height, const char* title, const DengineWindow* share)
{
    DengineWindow window;
    memset(&window, 0, sizeof (DengineWindow));

#ifdef DENGINE_WIN_X11
    Window root = DefaultRootWindow(x_dpy);
    window.x_swa.event_mask =
            ExposureMask |
            KeyPressMask | KeyReleaseMask |
            ButtonPressMask | ButtonReleaseMask |
            PointerMotionMask;
#ifdef DENGINE_CONTEXT_GLX
    int conf_vi_attr[]=
    {
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_DOUBLEBUFFER, True,
        GLX_DEPTH_SIZE, _win_depth
    };
    int n_conf;
    GLXFBConfig* conf = glXChooseFBConfig(x_dpy, 0, conf_vi_attr, &n_conf);
    if(!conf)
    {
        dengineutils_logging_log("ERROR::Cannot choose a valid GLXFBConfig");
        return NULL;
    }

//    XVisualInfo* vi = glXChooseVisual(x_dpy, 0, conf_vi_attr);
//    if(!vi)
//    {
//        dengineutils_logging_log("ERROR::Cannot choose a valid XVisual");
//        return NULL;
//    }

//    window.x_swa.colormap = XCreateColormap(x_dpy, root, vi->visual, AllocNone);
//    window.x_win = XCreateWindow(x_dpy, root, 0, 0, width, height, 0,
//                           vi->depth, InputOutput, vi->visual,
//                           CWColormap | CWEventMask, &window.x_swa);

    window.x_win = XCreateWindow(x_dpy, root, 0, 0, width, height, 0,
                           CopyFromParent, InputOutput, CopyFromParent,
                           CWEventMask, &window.x_swa);
    if(!window.x_win)
    {
        dengineutils_logging_log("ERROR::Cannot XCreateWindow");
        return NULL;
    }

    if(!glXCreateContextAttribsARB)
    {
        dengineutils_logging_log("ERROR::Cannot getproc glXCreateContextAttribsARB");
        return NULL;
    }

    int ctxattr[]=
    {
        GLX_CONTEXT_MAJOR_VERSION_ARB, _gl_max,
        GLX_CONTEXT_MINOR_VERSION_ARB, _gl_min,
        None
    };

    GLXContext ctx_shr = NULL;
    if(share)
        ctx_shr = share->glx_ctx;

    window.glx_ctx = glXCreateContextAttribsARB(x_dpy, conf[0], ctx_shr, True, ctxattr);
    //window.glx_ctx = glXCreateContext(x_dpy, vi, ctx_shr, GL_TRUE);
    if(!window.glx_ctx)
    {
        dengineutils_logging_log("ERROR::Cannot glXCreateContext");
        return NULL;
    }

//    XFree(vi);
    XFree(conf);
#elif defined(DENGINE_CONTEXT_EGL)
    window.x_win = XCreateWindow(x_dpy, root, 0, 0, width, height, 0,
                            CopyFromParent, InputOutput, CopyFromParent, CWEventMask,
                            &window.x_swa);
    if(!window.x_win )
    {
        dengineutils_logging_log("ERROR::Cannot XCreateWindow");
        return NULL;
    }

    window.egl_dpy = egl_dpy;
    EGLContext shr = EGL_NO_CONTEXT;
    if(share)
        shr = share->egl_ctx;

    if(!_dengine_window_egl_createctx(window.egl_dpy, &window.egl_sfc, &window.egl_ctx, shr, window.x_win))
    {
        dengineutils_logging_log("ERROR::WINDOW::Cannot create EGLContext");
        return NULL;
    }
#endif
    // x11 delete message. why on earth is there no simple exit event?
    XSetWMProtocols(x_dpy, window.x_win, &wm_delete, 1);
    XMapWindow(x_dpy, window.x_win);
    XStoreName(x_dpy, window.x_win, title);
#elif defined(DENGINE_WIN32)
    wchar_t title_wcs[256];

    mbstowcs(title_wcs, title, sizeof(title_wcs));
    window.win32_hwnd = CreateWindowExW(0,
                             wc_class,
                             title_wcs,
                             WS_OVERLAPPEDWINDOW,
                             0, 0, width, height,
                             NULL,
                             NULL,
                             wc.hInstance,
                             NULL);
    if(window.win32_hwnd == NULL)
    {
        return NULL;
    }
    window.width = width;
    window.height = height;
    if(share)
        window.win32_ctx_shr = share->win32_ctx;

    ShowWindow(window.win32_hwnd, SW_NORMAL);

    //setup wgl
    PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),    // size of this pfd
        1,                                // version number
        PFD_DRAW_TO_WINDOW |              // support window
        PFD_SUPPORT_OPENGL |              // support OpenGL
        PFD_DOUBLEBUFFER,                 // double buffered
        PFD_TYPE_RGBA,                    // RGBA type
        32,                               // 32-bit color depth
        0, 0, 0, 0, 0, 0,                 // color bits ignored
        0,                                // no alpha buffer
        0,                                // shift bit ignored
        0,                                // no accumulation buffer
        0, 0, 0, 0,                       // accum bits ignored
        24,                               // 24-bit z-buffer
        8,                                // no stencil buffer
        0,                                // no auxiliary buffer
        PFD_MAIN_PLANE,                   // main layer
        0,                                // reserved
        0, 0, 0
    };

    HDC hdc = GetDC(window.win32_hwnd);
    int pix = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pix, &pfd);
    HGLRC dummy = wglCreateContext(hdc);
    if(dummy == NULL)
    {
        MessageBox(window.win32_hwnd, "Cannot wglContext dummy. OpenGL probably not supported", "Error", MB_OK | MB_ICONEXCLAMATION);
        goto RelDCRetNULL;
    }else
    {
        if(!wglMakeCurrent(hdc, dummy))
        {
            MessageBox(window.win32_hwnd, "Cannot wglMakeCurrent dummy. OpenGL is really not supported", "Error", MB_OK | MB_ICONEXCLAMATION);
            goto RelDCRetNULL;
        }

        wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC) wglGetProcAddress("wglCreateContextAttribsARB");
        wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC) wglGetProcAddress("wglChoosePixelFormatARB");
        if(!wglChoosePixelFormatARB)
        {
            dengineutils_logging_log("WARNING::wglChoosePixelFormatARB NOT found. Cannot probably create \"modern\" contexts");
        }
        if(!wglCreateContextAttribsARB){
            dengineutils_logging_log("WARNING::wglCreateContextAttribsARB NOT found. Cannot really create \"modern\" contexts");
        }
    }

    if(wglChoosePixelFormatARB)
    {
        wglDeleteContext(dummy);
        dummy = NULL;
        const int pix_attrs[] =
        {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            WGL_COLOR_BITS_ARB, 32,
            WGL_DEPTH_BITS_ARB, _win_depth,
            //WGL_STENCIL_BITS_ARB, 8,
            0, // End
        };

        int pixelFormat;
        UINT numFormats;
        if(wglChoosePixelFormatARB(hdc, pix_attrs, NULL, 1, &pixelFormat, &numFormats) == FALSE)
        {
            MessageBox(window.win32_hwnd, "wglChoosePixelFormatARB failed", "Error", MB_OK | MB_ICONEXCLAMATION);
            goto RelDCRetNULL;
        }

        int ctx_attrs[]=
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, _gl_max,
            WGL_CONTEXT_MINOR_VERSION_ARB, _gl_min,
            0
        };

        window.win32_ctx = wglCreateContextAttribsARB(hdc, window.win32_ctx_shr, ctx_attrs);
        if(window.win32_ctx == NULL)
        {
            dengineutils_logging_log("WARNING::wglCreateContextAttribsARB failed");
            goto RelDCRetNULL;
        }else
        {
            wglMakeCurrent(hdc, window.win32_ctx);
            //get wglEXTS
            wglSwapIntervalEXT = wglGetProcAddress("wglSwapIntervalEXT");
            wglMakeCurrent(hdc, NULL);
        }
    }

    if(window.win32_ctx == NULL && dummy != NULL)
    {
        window.win32_ctx = dummy;
        dengineutils_logging_log("WARNING::Using dummy wglCreateContext Context");
    }

    ReleaseDC(window.win32_hwnd, hdc);
#elif defined(DENGINE_ANDROID)
    window.and_win = dengineutils_android_get_window();
    window.egl_dpy = egl_dpy;
    if(!_dengine_window_egl_createctx(window.egl_dpy, &window.egl_sfc, &window.egl_ctx, NULL, window.and_win))
    {
        dengineutils_logging_log("ERROR::WINDOW::Cannot create EGLContext");
        return NULL;
    }
#endif
    DengineWindow* ret = malloc(sizeof(DengineWindow));
    memcpy(ret, &window, sizeof(DengineWindow));
    ret->running = 1;
#ifdef DENGINE_WIN32
    //set lpParam
    SetWindowLongPtr(ret->win32_hwnd, GWLP_USERDATA, (LONG_PTR)ret);
#endif

    Thread input_thr;
    dengineutils_thread_create(_dengine_window_pollinf, ret, &input_thr);

    return ret;

#ifdef DENGINE_WIN32
    RelDCRetNULL:
        ReleaseDC(window.win32_hwnd, hdc);
        return NULL;
#endif
}

void dengine_window_destroy(DengineWindow* window)
{
#ifdef DENGINE_CONTEXT_GLX
    glXDestroyContext(x_dpy, window->glx_ctx);
//    XFreeColormap(x_dpy, window->x_swa.colormap);
#endif

#ifdef DENGINE_CONTEXT_EGL
    eglDestroyContext(window->egl_dpy, window->egl_ctx);
    eglDestroySurface(window->egl_dpy, window->egl_sfc);
#endif

#ifdef DENGINE_WIN_X11
   XUnmapWindow(x_dpy, window->x_win);
   XDestroyWindow(x_dpy, window->x_win);
#endif

#ifdef DENGINE_WIN32
   wglDeleteContext(window->win32_ctx);
   DestroyWindow(window->win32_hwnd);
#endif

   if(window->gl_lib)
        dengineutils_dynlib_close(window->gl_lib);

   free(window);
}

void dengine_window_get_dim(DengineWindow* window, int* width, int* height)
{
#ifdef DENGINE_WIN_X11
    XWindowAttributes attribs;
    XGetWindowAttributes(x_dpy, window->x_win,&attribs);
    if(width)
        *width = attribs.width;
    if(height)
        *height = attribs.height;
#elif defined(DENGINE_WIN32)
    RECT rect;
    GetWindowRect(window->win32_hwnd, &rect);
    if(width)
        *width = (int)(rect.right - rect.left);
    if(height)
        *height = (int)(rect.bottom - rect.top);
#elif defined(DENGINE_ANDROID)
    if(dengineutils_android_iswindowrunning()) {
        if(width)
            *width = (int)ANativeWindow_getWidth(window->and_win);
        if(height)
            *height = (int)ANativeWindow_getHeight(window->and_win);
    }
#endif
}

void dengine_window_swapbuffers(DengineWindow* window)
{
#ifdef DENGINE_CONTEXT_GLX
    GLXContext ctx = glXGetCurrentContext();
    if(ctx)
        glXSwapBuffers(x_dpy, window->x_win);
#elif defined (DENGINE_CONTEXT_EGL)
    eglSwapBuffers(window->egl_dpy, window->egl_sfc);
#elif defined (DENGINE_WIN32)
    HDC hdc = GetDC(window->win32_hwnd);
    SwapBuffers(hdc);
    ReleaseDC(window->win32_hwnd, hdc);
#endif
}

int dengine_window_isrunning(DengineWindow* window)
{
#ifdef DENGINE_ANDROID
    return dengineutils_android_iswindowrunning();
#else
    return window->running;
#endif
}

void* dengine_window_get_proc(const char* name)
{
    void* sym = NULL;
#ifdef DENGINE_CONTEXT_WGL
    sym = wglGetProcAddress(name);
#elif defined(DENGINE_CONTEXT_GLX)
    sym = glXGetProcAddress((GLubyte*) name);
#elif defined(DENGINE_CONTEXT_EGL)
    sym = eglGetProcAddress(name);
#endif

    if(
            sym == 0 ||
            sym == (void*)0x1 ||
            sym == (void*)0x2 ||
            sym == (void*)0x3 ||
            sym == (void*)-1)
    {
        sym = dengineutils_dynlib_get_sym(gl, name);
    }

    if(!sym)
        dengineutils_logging_log("WARNING::WINDOW::get_proc: %s not found", name);

    return sym;
}

int dengine_window_loadgl(DengineWindow* window)
{
    window->gl_lib = dengineutils_dynlib_open(GL);
    if(!window->gl_lib)
    {
        dengineutils_logging_log("WARNING::Could not find GL library [%s]. Some missing OpenGL functions may cause a crash", GL);
        return 0;
    }else
    {
        dengineutils_logging_log("INFO::Using GL library: %s", GL);
    }
    gl = window->gl_lib;
#ifdef DENGINE_ANDROID
    window->gl_load = gladLoadGLES2Loader((GLADloadproc)eglGetProcAddress);
#else
    window->gl_load = gladLoadGLLoader((GLADloadproc)dengine_window_get_proc);
#endif

    //one for all
//    _gl_load = gladLoadGLLoader((GLADloadproc)dengine_window_get_proc);


//#ifdef DENGINE_CONTEXT_GLX
//    _gl_load = gladLoadGLLoader((GLADloadproc)glXGetProcAddress);
//#elif defined (DENGINE_CONTEXT_EGL)
//#ifdef DENGINE_ANDROID
//    _gl_load = gladLoadGLES2Loader((GLADloadproc)eglGetProcAddress);
//#else
//    _gl_load = gladLoadGLLoader((GLADloadproc)eglGetProcAddress);
//#endif
//#elif defined(DENGINE_CONTEXT_WGL)
//    _gl_load = gladLoadGLLoader((GLADloadproc)dengine_window_get_proc);
//#endif

    return window->gl_load ;
}

int dengine_window_makecurrent(DengineWindow* window)
{
    int make = 0;
#ifdef DENGINE_CONTEXT_GLX
    make = glXMakeCurrent(x_dpy, window->x_win, window->glx_ctx);
#elif defined(DENGINE_CONTEXT_EGL)
    eglMakeCurrent(window->egl_dpy, window->egl_sfc, window->egl_sfc, window->egl_ctx);
    EGLContext cur = eglGetCurrentContext();
    if(cur != EGL_NO_CONTEXT)
        make = 1;
#elif defined(DENGINE_WIN32)
    HDC hdc = GetDC(window->win32_hwnd);
    wglMakeCurrent(hdc, window->win32_ctx);
    ReleaseDC(window->win32_hwnd, hdc);
    if(window->win32_ctx)
        make = 1;
#endif
    if(make)
    {
        current = window;
        dengine_input_set_window(current);
    }
    else{
        dengineutils_logging_log("WARNING::Unable to make current [%p]", window);
    }
    return make;
}

DengineWindow* dengine_window_get_current()
{
#ifdef DENGINE_ANDROID
    if(dengineutils_android_iswindowrunning())
        return current;
    else
        return NULL;
#else
    return current;
#endif
}

int dengine_window_set_swapinterval(DengineWindow* window, int interval)
{
    int itv = 0;
#ifdef DENGINE_CONTEXT_EGL
    itv = eglSwapInterval(window->egl_dpy, interval);
#elif defined(DENGINE_CONTEXT_GLX)
    if(glXSwapIntervalEXT)
    {
        glXSwapIntervalEXT(x_dpy, window->x_win, interval);
        itv = interval;
    }
    else{
        dengineutils_logging_log("WARNING::glXSwapInternalEXT unavailable");
    }
#elif defined(DENGINE_CONTEXT_WGL)
    if(wglSwapIntervalEXT)
    {
        wglSwapIntervalEXT(interval);
        itv = interval;
    }
    else{
        dengineutils_logging_log("WARNING::wglSwapIntervalEXT unavailable");
    }
    //wglSwapIntervalEXT
#endif
    return itv;
}

void* _dengine_window_pollinf(void* arg)
{
    DengineWindow* window = arg;
    while(window->running)
    {
#ifdef DENGINE_WIN_X11
        XEvent closeev;
        XPeekEvent(x_dpy, &closeev);
        if(closeev.type == ClientMessage)
        {
            if(closeev.xclient.data.l[0] == wm_delete)
            {
                window->running = 0;
            }
        }
#endif
    }
    return NULL;
}

int dengine_window_poll(DengineWindow* window)
{
    int polled = 0;
#ifdef DENGINE_WIN_X11
    int h;
    dengine_window_get_dim(window, NULL, &h);
    char key;
    KeySym keysym;

    if(XPending(x_dpy))
    {
        polled = XCheckWindowEvent(x_dpy, window->x_win,
                  window->x_swa.event_mask,
                  &window->ev);

        if(window->ev.type == Expose && window->gl_load)
        {
            int w,h;
            dengine_window_get_dim(window, &w, &h);
            dengine_viewport_set(0, 0, w, h);
        }

        if(window->ev.type == KeyPress)
        {
            XLookupString(&window->ev.xkey, &key, 1, &keysym, NULL);
            char up = toupper(key);
            _dengine_window_processkey(&window->input, up, 0);
        }

        if(window->ev.type == KeyRelease)
        {
            XLookupString(&window->ev.xkey, &key, 1, &keysym, NULL);
            char up = toupper(key);
            _dengine_window_processkey(&window->input, up, 1);
        }

        if(window->ev.type == ButtonPress)
        {
            if(window->ev.xbutton.button >= Button1 &&
                    window->ev.xbutton.button <= Button3)
            {
                window->input.msebtn[window->ev.xbutton.button - 1] = 1;
//                printf("press %u %u\n", window->ev.xbutton.button,
//                       window->ev.xbutton.state);
            }
        }

        if(window->ev.type == ButtonRelease)
        {
            if(window->ev.xbutton.button >= Button1 &&
                    window->ev.xbutton.button <= Button3)
            {
                window->input.msebtn[window->ev.xbutton.button - 1] = 0;
//                printf("press %u %u\n", window->ev.xbutton.button,
//                       window->ev.xbutton.state);
            }

            if(window->ev.xbutton.button == Button5)
            {
                window->input.msesrl_y = -1.0;
            }

            if(window->ev.xbutton.button == Button4)
            {
                window->input.msesrl_y = 1.0;
            }
        }

        if(window->ev.type == MotionNotify)
        {
            Window root, child;
            int x_root, y_root, x, y;
            uint32_t masks;
            XQueryPointer(x_dpy, window->x_win,
                          &root, &child,
                          &x_root, &y_root,
                          &x, &y,
                          &masks);

            window->input.mse_x = x;
            window->input.mse_y = h - y;
        }
    }

#elif defined(DENGINE_WIN32)
    PeekMessageW(&window->win32_msg, window->win32_hwnd, 0, 0, PM_REMOVE);
    TranslateMessage(&window->win32_msg);
    polled = DispatchMessageW(&window->win32_msg);
#elif defined(DENGINE_ANDROID)
    dengineutils_android_pollevents();
    if(window && dengineutils_android_iswindowrunning())
    {
        int h;
        dengine_window_get_dim(window, NULL, &h);
        AndroidInput* andr_input = dengineutils_android_get_input();
        if(andr_input->pointer0.state == 0)
        {
            window->input.mse_x = andr_input->pointer0.x;
            window->input.mse_y = (float)h - andr_input->pointer0.y;
            andr_input->pointer0.state = -1;
            if(window->input.msebtn[0] != -1)
                window->input.msebtn[0] = 1;
        }else
        {
            window->input.msebtn[0] = 0;
            window->input.mse_x = 0.0;
            window->input.mse_y = 0.0;
        }
    }
#endif

#ifdef DENGINE_HAS_GTK3
   gtk_main_iteration_do(0);
#endif

   return polled;
}

int dengine_window_resize(DengineWindow* window, int width, int height)
{
#ifdef DENGINE_WIN_X11
    return XResizeWindow(x_dpy, window->x_win, width, height);
#elif defined(DENGINE_WIN32)
    return MoveWindow(window->win32_hwnd, 0, 0, width, height, TRUE);
#else
    return 0;
#endif
}

void dengine_window_set_fullscreen(DengineWindow* window, int state)
{
#ifdef DENGINE_WIN_X11
    XWindowAttributes xwa;
    XGetWindowAttributes( x_dpy, window->x_win, &xwa );

    XEvent e = { 0 };
    e.xclient.type = ClientMessage;
    e.xclient.message_type = XInternAtom( x_dpy, "_NET_WM_STATE", False );
    e.xclient.display = x_dpy;
    e.xclient.window = window->x_win;
    e.xclient.format = 32;
    e.xclient.data.l[0] = state;
    e.xclient.data.l[1] = XInternAtom( x_dpy, "_NET_WM_STATE_FULLSCREEN", False );
    XSendEvent( x_dpy, xwa.root,
        False, SubstructureNotifyMask | SubstructureRedirectMask, &e );
#elif defined(DENGINE_WIN32)
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    HWND after = HWND_TOP;
    DWORD style = WS_POPUP;
    if(!state)
    {
        style = WS_OVERLAPPEDWINDOW;
        after = NULL;
        w = window->width;
        h = window->height;
    }
    window->win32_after = after;

    SetWindowLongPtr(window->win32_hwnd, GWL_STYLE, WS_VISIBLE | style);
    SetWindowPos(window->win32_hwnd, after, 0, 0, w, h, SWP_FRAMECHANGED);
#endif
}

int dengine_window_set_position(DengineWindow* window, int x, int y)
{
    int set = 0;
#ifdef DENGINE_WIN_X11
    XWindowChanges changes;
    changes.x = x;
    changes.y = y;
    set = XConfigureWindow(x_dpy, window->x_win, CWX | CWY, &changes);
#elif defined(DENGINE_WIN32)
    set = SetWindowPos(window->win32_hwnd, window->win32_after, x, y,
                 window->width, window->height, SWP_FRAMECHANGED);
#endif
    return set;
}

/* PLATFORM SPECIFICS */

#ifdef DENGINE_CONTEXT_EGL
int _dengine_window_egl_createctx(EGLDisplay dpy, EGLSurface* egl_sfc, EGLContext* egl_ctx, EGLContext share, EGLNativeWindowType win)
{
    //EGL spec we want
    EGLint egl_spec_desired[] ={
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_RED_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_BLUE_SIZE, 8,
                EGL_ALPHA_SIZE, 8,
                EGL_DEPTH_SIZE, _win_depth,
                EGL_SAMPLES, _win_msaa,
                EGL_NONE
    };
    //Get config count
    EGLint supported_count;
    eglChooseConfig(dpy, egl_spec_desired, NULL, 0, &supported_count);

    if(supported_count == 0)
    {
        dengineutils_logging_log("ERROR::WINDOW::NO_SUITABLE_EGL_CONFIG_FOUND!");
        return 0;
    }

    //Now get an actual config
    EGLConfig configs[supported_count];
    eglChooseConfig(dpy, egl_spec_desired, configs, supported_count, &supported_count);

    *egl_sfc = eglCreateWindowSurface(dpy, configs[0], win, NULL);
    if(*egl_sfc == EGL_NO_SURFACE){
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
    *egl_ctx = eglCreateContext(dpy, configs[0], share, egl_context_attribs);
    if(*egl_ctx == EGL_NO_CONTEXT){
        dengineutils_logging_log("ERROR::WINDOW::NO_EGL_CONTEXT!");
        return 0;
    }

    return 1;
}
#endif

#ifdef DENGINE_WIN32
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    DengineWindow* window;
    if(uMsg == WM_CREATE)
    {
        CREATESTRUCT* create = (CREATESTRUCT*)lParam;
        window = create->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);
    }else
    {
        window = (DengineWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    char key[MB_CUR_MAX];
    wctomb(key, wParam);

    switch (uMsg) {

    case WM_SIZE:{
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        if(window->gl_load)
            dengine_viewport_set(0, 0, width, height);
        return 0;
    }

    case WM_PAINT:{
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_CLOSE:
    {
//        if(MessageBoxW(hwnd, L"Do you want to quit?", L"Dengine", MB_OKCANCEL) == IDOK)
//        {
            window->running = 0;
            return 0;
//        }
    }

    case WM_LBUTTONDOWN:
    {
        if(window->input.msebtn[DENGINE_INPUT_MSEBTN_PRIMARY] != -1)
            window->input.msebtn[DENGINE_INPUT_MSEBTN_PRIMARY] = 1;
        return 0;
    }
    case WM_LBUTTONUP:
    {
        window->input.msebtn[DENGINE_INPUT_MSEBTN_PRIMARY] = 0;
        return 0;
    }
    case WM_RBUTTONDOWN:
    {
        if(window->input.msebtn[DENGINE_INPUT_MSEBTN_SECONDARY] != -1)
            window->input.msebtn[DENGINE_INPUT_MSEBTN_SECONDARY] = 1;
        return 0;
    }
    case WM_RBUTTONUP:
    {
        window->input.msebtn[DENGINE_INPUT_MSEBTN_SECONDARY] = 0;
        return 0;
    }
    case WM_MBUTTONDOWN:
    {
        if(window->input.msebtn[DENGINE_INPUT_MSEBTN_MIDDLE] != -1)
            window->input.msebtn[DENGINE_INPUT_MSEBTN_MIDDLE] = 1;
        return 0;
    }
    case WM_MBUTTONUP:
    {
        window->input.msebtn[DENGINE_INPUT_MSEBTN_MIDDLE] = 0;
        return 0;
    }

    case WM_MOUSEMOVE:
    {
        int h;
        dengine_window_get_dim(window, NULL, &h);
        window->input.mse_x = GET_X_LPARAM(lParam);
        window->input.mse_y = h - GET_Y_LPARAM(lParam);
        return 0;
    }

    case WM_KEYUP:
    {
        _dengine_window_processkey(&window->input, key[0], 1);
        return 0;
    }

    case WM_KEYDOWN:
    {
        _dengine_window_processkey(&window->input, key[0], 0);
        return 0;
    }

    default:{
        return DefWindowProcW(hwnd, uMsg, wParam, lParam);;
    }

    }
}

#endif

WindowInput* dengine_window_get_input(DengineWindow* window)
{
    return &window->input;
}
