#include "dengine/window.h"

#include "dengine/viewport.h" //set_view
#include "dengine/loadgl.h" //glad

#include "dengine-utils/logging.h"//log
#include "dengine-utils/dynlib.h" //getsym

#include <stdio.h>  //printf
#include <stdlib.h> //malloc
#include <string.h> //memcpy
#include <ctype.h> //toupper

//WINDOW CREATION INCL.
#ifdef DENGINE_WIN_X11
#include <X11/Xlib.h> //Window
#elif defined (DENGINE_WIN32)
#include <windows.h> //HWND
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

#ifdef DENGINE_LINUX
#define GL "/lib/libGL.so"
#elif defined(DENGINE_WIN32)
#define GL "C:\\windows\\system32\\opengl32.dll"
#elif defined(DENGINE_ANDROID)
#define GL "/system/lib/libGLESv3.so"
#endif

#ifdef DENGINE_WIN32
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

#ifdef DENGINE_CONTEXT_EGL
int _dengine_window_egl_createctx(EGLDisplay dpy, EGLSurface* egl_sfc, EGLContext* egl_ctx, EGLContext share, EGLNativeWindowType win);
#endif
struct _DengineWindow
{
    ContextType type;
    int running;
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

    WindowInput input;
};
#define DFT_GL_MAX 2
#define DFT_GL_MIN 0
#define DFT_WIN_MSAA 0
#define DFT_WIN_DEPTH 24
static int _gl_max = DFT_GL_MAX, _gl_min = DFT_GL_MIN; /*_gl_core = 0 , */
static int _win_msaa = DFT_WIN_MSAA, _win_depth = DFT_WIN_DEPTH;
//static int _win_ctx = 0;
static int _gl_load = 0;


#ifdef DENGINE_WIN_X11
Display* x_dpy;
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
PFNGLGETSTRINGIPROC wglGetStringi = NULL;
#endif
#ifdef DENGINE_WIN32
WNDCLASSW wc = { };
static const wchar_t* wc_class = L"Dengine_Win32";
#endif

DengineWindow current;
DynLib gl = NULL;

int dengine_window_init()
{
    int init = 0;
    gl = dengineutils_dynlib_open(GL);
#ifdef DENGINE_WIN_X11
    x_dpy = XOpenDisplay(NULL);
    if(x_dpy)
    {
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
    current.egl_dpy = egl_dpy;
    ANativeWindow* android_native_window = dengineutils_android_get_window();
    if(!_dengine_window_egl_createctx(current.egl_dpy, &current.egl_sfc, &current.egl_ctx, android_native_window));
        dengineutils_logging_log("ERROR::WINDOW::Cannot create EGLContext");

    if(!eglMakeCurrent(current.egl_dpy, current.egl_sfc, current.egl_sfc, current.egl_ctx))
        dengineutils_logging_log("ERROR::WINDOW::CANNOT_MAKE_CONTEXT_CURRENT!");
#endif
    return init;
}

void dengine_window_terminate()
{
#ifdef DENGINE_WIN_X11
   XCloseDisplay(x_dpy);
#endif
#ifdef DENGINE_WIN32
    if(gl)
        dengineutils_dynlib_close(gl);
    UnregisterClassW(wc_class, wc.hInstance);
#endif
#ifdef DENGINE_CONTEXT_EGL
   eglTerminate(egl_dpy);
#endif
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
#ifdef DENGINE_WIN_X11
    DengineWindow window;
    memset(&window, 0, sizeof (DengineWindow));

    Window root = DefaultRootWindow(x_dpy);
    window.x_swa.event_mask =
            ExposureMask |
            KeyPressMask | KeyReleaseMask |
            ButtonPressMask | ButtonReleaseMask |
            PointerMotionMask;

    //disable annoying repeat
    //XAutoRepeatOff(x_dpy);

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

    XVisualInfo* vi = glXChooseVisual(x_dpy, 0, conf_vi_attr);
    if(!vi)
    {
        dengineutils_logging_log("ERROR::Cannot choose a valid XVisual");
        return NULL;
    }

    window.x_swa.colormap = XCreateColormap(x_dpy, root, vi->visual, AllocNone);
    window.x_win = XCreateWindow(x_dpy, root, 0, 0, width, height, 0,
                           vi->depth, InputOutput, vi->visual,
                           CWColormap | CWEventMask, &window.x_swa);

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
    XMapWindow(x_dpy, window.x_win);
    XStoreName(x_dpy, window.x_win, title);

    DengineWindow* ret = malloc(sizeof(DengineWindow));
    memcpy(ret, &window, sizeof(DengineWindow));
    ret->running = 1;
    return ret;
#elif defined(DENGINE_WIN32)
    wchar_t title_wcs[256];

    mbstowcs(title_wcs, title, sizeof(title_wcs));
    DengineWindow* ret = calloc(1, sizeof(DengineWindow));
    ret->win32_hwnd = CreateWindowExW(0,
                             wc_class,
                             title_wcs,
                             WS_OVERLAPPEDWINDOW,
                             0, 0, width, height,
                             NULL,
                             NULL,
                             wc.hInstance,
                             ret);
    if(ret->win32_hwnd == NULL)
    {
        free(ret);
        return NULL;
    }
    ret->running = 1;
    ShowWindow(ret->win32_hwnd, SW_NORMAL);
    return ret;
#endif
}

void dengine_window_destroy(DengineWindow* window)
{
#ifdef DENGINE_CONTEXT_GLX
  glXDestroyContext(x_dpy, window->glx_ctx);
#endif

#ifdef DENGINE_WIN_X11
   XUnmapWindow(x_dpy, window->x_win);
   XDestroyWindow(x_dpy, window->x_win);
#endif

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
    if(width)
        *width = (int)ANativeWindow_getWidth(window->and_window);
    if(height)
        *height = (int)ANativeWindow_getHeight(window->and_window);
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
    return window->running;
//    #ifndef DENGINE_GL_NONE
//    if(glinit)
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    #endif // DENGINE_GL_NONE
//    return isrunning;
}

void* dengine_window_get_proc(const char* name)
{
    /*
  void *p = (void *)wglGetProcAddress(name);
  if(p == 0 ||
    (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) ||
    (p == (void*)-1) )
  {
    HMODULE module = LoadLibraryA("opengl32.dll");
    p = (void *)GetProcAddress(module, name);
  }
     */

    void* sym = dengineutils_dynlib_get_sym(gl, name);
#ifdef DENGINE_CONTEXT_WGL
    //special case for glGetStringi in wgl not being exported to openg32.dll for some reason
    if(!strcmp(name, "glGetStringi") && !sym)
    {
        if(!wglGetStringi)
            MessageBox(NULL, "wglGetStringi failed", "Critical Error", MB_OK | MB_ICONERROR);
        sym = wglGetStringi;
        dengineutils_logging_log("WARNING::Redirected %s to wglGetStringi", name);
    }
#endif
    return sym;
}

int dengine_window_loadgl(DengineWindow* window)
{
#ifdef DENGINE_ANDROID
    _gl_load = gladLoadGLES2Loader((GLADloadproc)eglGetProcAddress);
#else
    _gl_load = gladLoadGLLoader((GLADloadproc)dengine_window_get_proc);
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

    return _gl_load;
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
    return make;
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

int dengine_window_poll(DengineWindow* window)
{
    int polled = 0;
#ifdef DENGINE_WIN_X11
    char key;
    KeySym keysym;
    if(XPending(x_dpy))
    {
        polled = XCheckWindowEvent(x_dpy, window->x_win,
                  window->x_swa.event_mask,
                  &window->ev);
        if(window->ev.type == Expose && _gl_load)
        {
            int w,h;
            dengine_window_get_dim(window, &w, &h);
            dengine_viewport_set(0, 0, w, h);
        }

        if(window->ev.type == KeyPress)
        {
            XLookupString(&window->ev.xkey, &key, 1, &keysym, NULL);
            char up = toupper(key);
            if(up >= 33 && up <= 126)
            {
                for (int i = 0; i < DENGINE_WINDOW_ALPNUM; i++) {
                    //dont dupe;
                    if(window->input.alpnum[i] == up)
                        break;

                    if(window->input.alpnum[i] != up && window->input.alpnum[i] == 0)
                    {
                        window->input.alpnum[i] = up;
                        break;
                    }
                }
//                for (int i = 0; i < DENGINE_WINDOW_ALPNUM; i++)
//                {
//                    printf("[%c] ", window->input.alpnum[i]);
//                }
//                printf("\n");
            }
        }

        if(window->ev.type == KeyRelease)
        {
            XLookupString(&window->ev.xkey, &key, 1, &keysym, NULL);
            char up = toupper(key);
            if(up >= 33 && up <= 126)
            {
                for (int i = 0; i < DENGINE_WINDOW_ALPNUM; i++) {
                    if(window->input.alpnum[i] == up)
                    {
                        window->input.alpnum[i] = 0;
                        break;
                    }
                }
//                for (int i = 0; i < DENGINE_WINDOW_ALPNUM; i++)
//                {
//                    printf("[%c] ", window->input.alpnum[i]);
//                }
//                printf("\n");
            }
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
        }

        if(window->ev.xbutton.button == Button5)
        {
            window->input.msesrl_y = -1.0;
        }

        if(window->ev.xbutton.button == Button4)
        {
            window->input.msesrl_y = 1.0;
        }

        if(window->ev.type == MotionNotify)
        {
            int h;
            dengine_window_get_dim(window, NULL, &h);

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
    GetMessageW(&window->win32_msg, window->win32_hwnd, 0, 0);
    TranslateMessage(&window->win32_msg);
    polled = DispatchMessageW(&window->win32_msg);
#elif defined(DENGINE_ANDROID)
    dengineutils_android_pollevents();
#endif

#ifdef DENGINE_HAS_GTK3
   gtk_main_iteration_do(0);
#endif

   return polled;
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
    switch (uMsg) {

    case WM_SIZE:{
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        if(_gl_load)
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
        if(MessageBoxW(hwnd, L"Do you want to quit?", L"App", MB_OKCANCEL) == IDOK)
        {
            DengineWindow* win = (DengineWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            win->running = 0;
            DestroyWindow(hwnd);
            return 0;
        }
    }

    case WM_CREATE:{
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

        CREATESTRUCT* create = (CREATESTRUCT*)lParam;
        DengineWindow* w = create->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)w);
        DengineWindow* win = (DengineWindow*) GetWindowLongPtr(hwnd, GWLP_USERDATA);

        HDC hdc = GetDC(hwnd);
        int pix = ChoosePixelFormat(hdc, &pfd);
        SetPixelFormat(hdc, pix, &pfd);
        HGLRC dummy = wglCreateContext(hdc);
        if(!dummy)
        {
            MessageBox(hwnd, "Cannot wglContext", "Error", MB_OK | MB_ICONEXCLAMATION);
        }else
        {
            if(!wglMakeCurrent(hdc, dummy))
                MessageBox(hwnd, "Cannot wglMakeCurrent dummy", "Error", MB_OK | MB_ICONEXCLAMATION);

            wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC) wglGetProcAddress("wglCreateContextAttribsARB");
            wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC) wglGetProcAddress("wglChoosePixelFormatARB");
            if(!wglChoosePixelFormatARB)
                MessageBox(hwnd, "wglChoosePixelFormatARB NOT found", "Error", MB_OK | MB_ICONEXCLAMATION);
            if(!wglCreateContextAttribsARB)
                MessageBox(hwnd, "wglCreateContextAttribsARB NOT found", "Error", MB_OK | MB_ICONEXCLAMATION);
        }

        if(wglChoosePixelFormatARB)
        {
            wglDeleteContext(dummy);
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
                MessageBox(hwnd, "wglChoosePixelFormatARB failed", "Error", MB_OK | MB_ICONEXCLAMATION);
            }

            int ctx_attrs[]=
            {
                WGL_CONTEXT_MAJOR_VERSION_ARB, _gl_max,
                WGL_CONTEXT_MINOR_VERSION_ARB, _gl_min,
                0
            };

            win->win32_ctx = wglCreateContextAttribsARB(hdc, win->win32_ctx_shr, ctx_attrs);
            if(!win->win32_ctx)
            {
                MessageBox(hwnd, "wglCreateContextAttribsARB failed", "Error", MB_OK | MB_ICONEXCLAMATION);
            }else
            {
                wglMakeCurrent(hdc, win->win32_ctx);
                //get wglEXTS
                wglSwapIntervalEXT = wglGetProcAddress("wglSwapIntervalEXT");
                wglGetStringi = (PFNGLGETSTRINGIPROC) wglGetProcAddress("glGetStringi");
                wglMakeCurrent(hdc, NULL);
            }
        }else
        {
            win->win32_ctx = dummy;
            dengineutils_logging_log("WARNING::Using dummy wglCreateContext Context");
        }
        ReleaseDC(hwnd, hdc);

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
