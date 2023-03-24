#include "dengine/window.h"

#include "dengine-core_internal.h" /* gamepad_poll,input_terminate/init */

#include "dengine/viewport.h" //set_view
#include "dengine/loadgl.h" //GLADloadproc
#include "dengine/input.h" //setwindow
#include "dengine-utils/logging.h"//log
#include "dengine-utils/dynlib.h" //getsym
#include "dengine-utils/thread.h"
#include "dengine-utils/macros.h" //ARY_SZ
#include "dengine-utils/timer.h"
#include "dengine-utils/debug.h"

#include <stdio.h>  //printf
#include <stdlib.h> //malloc
#include <string.h> //memcpy
#include <ctype.h> //toupper

//WINDOW CREATION INCL.
#ifdef DENGINE_WIN_X11
#include <X11/Xlib.h> //Window
#include <X11/Xutil.h> /* XLookupString */
#include <X11/XKBlib.h>
#elif defined (DENGINE_WIN32)
#include <windows.h> //HWND
#include <windowsx.h> //GET_X_LPARAM
#elif defined (DENGINE_ANDROID)
#include <dengine-utils/platform/android.h> //ANativeWindow
#elif defined(DENGINE_WIN_WAYLAND)
#include <wayland-client.h>
#include <wayland-egl.h>
#include "xdg-shell.xml.h"
#include <linux/input-event-codes.h> /* BTN_... */
#include <sys/mman.h> /* mmap */
#include <unistd.h> /* close */
#include <xkbcommon/xkbcommon.h>
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
#ifdef DENGINE_WIN_X11
int _dengine_window_x11err(Display* dpy, XErrorEvent* err);
#endif
#ifdef DENGINE_WIN_WAYLAND
static void registry_global(void* data, struct wl_registry* registry, uint32_t name, const char* ifc, uint32_t version);
static void registry_global_remove(void* data, struct wl_registry* registry, uint32_t name);

static void wm_base_ping(void* data, struct xdg_wm_base* wm, uint32_t serial);

static void xdg_surface_configure(void* data, struct xdg_surface* xdg_sfc, uint32_t serial);

static void xdg_toplevel_configure(void* data, struct xdg_toplevel* toplvl, int w, int h, struct wl_array* states);

static void xdg_toplevel_close(void* data, struct xdg_toplevel* toplvl);
#if XDG_TOPLEVEL_CONFIGURE_BOUNDS_SINCE_VERSION >= 4
static void xdg_toplevel_configure_bounds(void *data,struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height);
#endif
#if XDG_TOPLEVEL_WM_CAPABILITIES_SINCE_VERSION >= 5
static void xdg_toplevel_wmcap(void *data, struct xdg_toplevel *xdg_toplevel, struct wl_array *capabilities);
#endif
static void wl_seat_name(void *data, struct wl_seat *wl_seat, const char *name);
static void wl_seat_caps(void* data, struct wl_seat* seat, uint32_t caps);

static void wl_pointer_enter(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t x, wl_fixed_t y);
static void wl_pointer_motion(void *data, struct wl_pointer *wl_pointer, uint32_t time, wl_fixed_t x, wl_fixed_t y);
static void wl_pointer_frame(void* data, struct wl_pointer* pointer);
static void wl_pointer_button(void *data, struct wl_pointer *wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
static void wl_pointer_leave(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface);
static void wl_pointer_axis(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value);
static void wl_pointer_axis_stop(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis);
static void wl_pointer_axis_source(void *data, struct wl_pointer *wl_pointer, uint32_t axis_source);
static void wl_pointer_axis_discrete(void *data, struct wl_pointer *wl_pointer, uint32_t axis, int32_t discrete);

static void wl_keyboard_keymap(void *data, struct wl_keyboard *wl_keyboard, uint32_t format, int32_t fd, uint32_t size);
static void wl_keyboard_enter(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys);
static void wl_keyboard_leave(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface);
static void wl_keyboard_mods(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group);
static void wl_keyboard_key(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
static void wl_keyboard_repeatinfo (void *data, struct wl_keyboard *wl_keyboard, int32_t rate, int32_t delay);
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
#ifdef DENGINE_WIN_WAYLAND
   struct wl_surface* wl_sfc;
   struct xdg_surface* xdg_sfc;
   struct xdg_toplevel* xdg_top;
   struct wl_egl_window* wl_egl_win;
#endif
    int gl_load;
    DynLib gl_lib;
    WindowInput input;
    Thread* windowthr;

    Condition pollcond;
    int deref;
};

typedef struct
{
    int width;
    int height;
    const char* title;
    const DengineWindow* share;
    DengineWindow* ret;
    Condition* ret_cond;
    int* deref_and_set_to_one;
}CreateWindowAttrs;

#define DFT_GL_MAX 2
#define DFT_GL_MIN 0
#define DFT_WIN_MSAA 0
#define DFT_WIN_DEPTH 24
static int _gl_max = DFT_GL_MAX, _gl_min = DFT_GL_MIN, _gl_core = 0;
static int _win_msaa = DFT_WIN_MSAA, _win_depth = DFT_WIN_DEPTH;


#ifdef DENGINE_WIN_X11
Display* x_dpy;
Atom wm_delete;
#endif
#ifdef DENGINE_WIN_WAYLAND
static const struct wl_registry_listener registry_listener =
{
    .global = registry_global,
    .global_remove = registry_global_remove
};

static const struct xdg_wm_base_listener wm_base_listener =
{
    .ping = wm_base_ping
};

static const struct xdg_surface_listener xdg_sfc_listener =
{
    .configure = xdg_surface_configure
};

static const struct xdg_toplevel_listener xdg_toplevel_listener =
{
    .configure = xdg_toplevel_configure,
    .close = xdg_toplevel_close,
#if XDG_TOPLEVEL_CONFIGURE_BOUNDS_SINCE_VERSION >= 4
    .configure_bounds = xdg_toplevel_configure_bounds,
#endif
#if XDG_TOPLEVEL_WM_CAPABILITIES_SINCE_VERSION >= 5
    .wm_capabilities = xdg_toplevel_wmcap,
#endif
};

static const struct wl_seat_listener wl_seat_listener =
{
    .capabilities = wl_seat_caps,
    .name = wl_seat_name
};

static const struct wl_pointer_listener wl_ptr_listener =
{
    .enter = wl_pointer_enter,
    .motion = wl_pointer_motion,
    .frame = wl_pointer_frame,
    .leave = wl_pointer_leave,
    .button = wl_pointer_button,
    .axis = wl_pointer_axis,
    .axis_source = wl_pointer_axis_source,
    .axis_discrete = wl_pointer_axis_discrete,
    .axis_stop = wl_pointer_axis_stop
};

static const struct wl_keyboard_listener wl_kbd_listener =
{
    .keymap = wl_keyboard_keymap,
    .enter = wl_keyboard_enter,
    .leave = wl_keyboard_leave,
    .modifiers = wl_keyboard_mods,
    .key = wl_keyboard_key,
    .repeat_info = wl_keyboard_repeatinfo
};

struct wl_compositor* wl_comp = NULL;
struct wl_display* wl_dpy = NULL;
struct wl_registry* wl_registry = NULL;
struct xdg_wm_base* wm_base = NULL;
struct wl_output* wl_out = NULL;
struct wl_seat* wl_seat = NULL;
struct wl_pointer* wl_ptr = NULL;

struct xkb_context* xkb_ctx = NULL;
struct xkb_keymap* xkb_keymap = NULL;
struct xkb_state* xkb_state = NULL;
struct wl_keyboard* wl_kbd = NULL;
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
    DENGINE_DEBUG_ENTER;

    int init = 0;
#ifdef DENGINE_WIN_X11
    XInitThreads();
    x_dpy = XOpenDisplay(NULL);
    if(x_dpy)
    {
        wm_delete = XInternAtom(x_dpy, "WM_DELETE_WINDOW", False);
        /* error callback to prevent unwarranted crashes */
        XSetErrorHandler(_dengine_window_x11err);

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
#elif defined(DENGINE_WIN_WAYLAND)
    wl_dpy = wl_display_connect(NULL);
    if(wl_dpy == NULL)
    {
        dengineutils_logging_log("ERROR::WINDOW::Cannot open wl_display");
    }else
    {
        egl_dpy = eglGetDisplay(wl_dpy);
        if(egl_dpy == EGL_NO_DISPLAY){
            dengineutils_logging_log("ERROR::WINDOW::Cannot eglGetDisplay wl_display");
        }
        else
        {
            init = 1;

            wl_registry = wl_display_get_registry(wl_dpy);
            wl_registry_add_listener(wl_registry, &registry_listener, NULL);

            wl_display_dispatch(wl_dpy);
            wl_display_roundtrip(wl_dpy);

            if(wl_comp == NULL)
            {
                dengineutils_logging_log("ERROR::cannot find wl_compositor");
                init = 0;
            }

            if(wm_base == NULL)
            {
                dengineutils_logging_log("ERROR::cannot find xdg_wm_base");
                init = 0;
            }

            xkb_ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
            if(xkb_ctx == NULL)
            {
                dengineutils_logging_log("ERROR::cannot xkb_context_new");
                init = 0;
            }
        }
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
    _dengine_input_init();
    return init;
}

void dengine_window_terminate()
{
    DENGINE_DEBUG_ENTER;

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
#ifdef DENGINE_WIN_WAYLAND
    xkb_state_unref(xkb_state);
    xkb_keymap_unref(xkb_keymap);
    xkb_context_unref(xkb_ctx);
    wl_registry_destroy(wl_registry);
    wl_display_disconnect(wl_dpy);
#endif

    _dengine_input_terminate();
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
    DENGINE_DEBUG_ENTER;

    _gl_max = gl_major;
    _gl_min = gl_minor;
    _gl_core = gl_core;
}

void dengine_window_request_MSAA(int samples)
{
    DENGINE_DEBUG_ENTER;

    _win_msaa = samples;
}

void dengine_window_request_defaultall()
{
    DENGINE_DEBUG_ENTER;

    _gl_max = DFT_GL_MAX;
    _gl_min = DFT_GL_MIN;
    _win_msaa = DFT_WIN_MSAA;
    _win_depth = DFT_WIN_DEPTH;
    _gl_core = 0;
}

void* _dengine_window_createandpoll(void* args)
{
    CreateWindowAttrs* attrs = args;
    int width = attrs->width;
    int height = attrs->height;
    const char* title = attrs->title;
    const DengineWindow* share = attrs->share;

    DengineWindow window;
    memset(&window, 0, sizeof (DengineWindow));
    uint32_t prof = 0;
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

    prof = GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
    if(_gl_core)
        prof = GLX_CONTEXT_CORE_PROFILE_BIT_ARB;
    int ctxattr[]=
    {
        GLX_CONTEXT_MAJOR_VERSION_ARB, _gl_max,
        GLX_CONTEXT_MINOR_VERSION_ARB, _gl_min,
        GLX_CONTEXT_PROFILE_MASK_ARB, prof,
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

    ShowWindow(window.win32_hwnd , SW_NORMAL);

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

        prof = WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
        if(_gl_core)
            prof = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
        int ctx_attrs[]=
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, _gl_max,
            WGL_CONTEXT_MINOR_VERSION_ARB, _gl_min,
            WGL_CONTEXT_PROFILE_MASK_ARB, prof,
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
#elif defined(DENGINE_WIN_WAYLAND)
    window.wl_sfc = wl_compositor_create_surface(wl_comp);
    if(window.wl_sfc == NULL)
    {
        dengineutils_logging_log("ERROR::cannot wl_compositor_create_surface");
        return NULL;
    }

    window.xdg_sfc = xdg_wm_base_get_xdg_surface(wm_base, window.wl_sfc);
    xdg_surface_add_listener(window.xdg_sfc, &xdg_sfc_listener, NULL);

    window.xdg_top = xdg_surface_get_toplevel(window.xdg_sfc);
    xdg_toplevel_set_title(window.xdg_top, title);
    xdg_toplevel_add_listener(window.xdg_top, &xdg_toplevel_listener, NULL);

    wl_surface_commit(window.wl_sfc);

    struct wl_region* opaque = wl_compositor_create_region(wl_comp);
    wl_region_add(opaque, 0, 0, width, height);
    wl_surface_set_opaque_region(window.wl_sfc, opaque);
    wl_region_destroy(opaque);

    window.wl_egl_win = wl_egl_window_create(window.wl_sfc, width, height);
    if(window.wl_egl_win == EGL_NO_SURFACE)
    {
        dengineutils_logging_log("ERROR::cannot wl_egl_window_create");
        return NULL;
    }

    window.egl_dpy = egl_dpy;
    EGLContext shr = EGL_NO_CONTEXT;
    if(share)
        shr = share->egl_ctx;

    if(!_dengine_window_egl_createctx(window.egl_dpy, &window.egl_sfc, &window.egl_ctx, shr, window.wl_egl_win))
    {
        dengineutils_logging_log("ERROR::WINDOW::Cannot _dengine_window_egl_createctx");
        return NULL;
    }

    window.width = width;
    window.height = height;
#endif
    DengineWindow* ret = calloc(1, sizeof(DengineWindow));
    memcpy(ret, &window, sizeof(DengineWindow));
    ret->running = 1;
#ifdef DENGINE_WIN32
    //set lpParam
    SetWindowLongPtr(ret->win32_hwnd, GWLP_USERDATA, (LONG_PTR)ret);
#endif
#ifdef DENGINE_WIN_WAYLAND
   /* set user datum (data plural?) */
    xdg_toplevel_set_user_data(ret->xdg_top, ret);
    wl_surface_set_user_data(ret->wl_sfc, ret);

    /* needs this round trip before event loop to
     * configure xdg_surface*/
    wl_display_roundtrip(wl_dpy);
#endif

    /* create out poll cond */
    dengineutils_thread_condition_create(&ret->pollcond);

    /* raise our cond last so main thread
     * gets hold of created window
     * from here control is handed to main thread, so
     * dont use attrs again since its been popped from stack
     * */
    attrs->ret = ret;
    *attrs->deref_and_set_to_one = 1;
    dengineutils_thread_condition_raise(attrs->ret_cond);

    while (ret->running) {
        dengineutils_thread_condition_wait(&ret->pollcond,
                                           &ret->deref);
        ret->deref = 0;
        _dengine_input_gamepad_poll();
        _dengine_window_pollinf(ret);
    }
    return NULL;

#ifdef DENGINE_WIN32
    RelDCRetNULL:
        ReleaseDC(window.win32_hwnd, hdc);
        return NULL;
#endif
}

DengineWindow* dengine_window_create(int width, int height, const char* title, const DengineWindow* share)
{
    DENGINE_DEBUG_ENTER;

    CreateWindowAttrs attrs;
    memset(&attrs, 0, sizeof(CreateWindowAttrs));

    attrs.width = width;
    attrs.height = height;
    attrs.title = title;
    attrs.share = share;
    Condition ret_cond;
    dengineutils_thread_condition_create(&ret_cond);
    attrs.ret_cond = &ret_cond;
    int oned = 0;
    attrs.deref_and_set_to_one = &oned;

    Thread* windowthr = calloc(1, sizeof(Thread));
    dengineutils_thread_create(_dengine_window_createandpoll, &attrs, windowthr);
    dengineutils_thread_condition_wait(&ret_cond, &oned);

    dengineutils_thread_condition_destroy(&ret_cond);
    attrs.ret->windowthr = windowthr;

    return attrs.ret;
}

void dengine_window_destroy(DengineWindow* window)
{
    DENGINE_DEBUG_ENTER;

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

#ifdef DENGINE_WIN_WAYLAND
   wl_egl_window_destroy(window->wl_egl_win);
   xdg_toplevel_destroy(window->xdg_top);
   xdg_surface_destroy(window->xdg_sfc);
   wl_surface_destroy(window->wl_sfc);
#endif

   if(window->gl_lib)
        dengineutils_dynlib_close(window->gl_lib);

   dengineutils_thread_wait(window->windowthr);
   dengineutils_thread_condition_destroy(&window->pollcond);

   free(window->windowthr);
   free(window);
}

void dengine_window_get_dim(DengineWindow* window, int* width, int* height)
{
    DENGINE_DEBUG_ENTER;

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
#elif defined(DENGINE_WIN_WAYLAND)
    if(width)
        *width = window->width;
    if(height)
        *height = window->height;
#endif
}

void dengine_window_swapbuffers(DengineWindow* window)
{
    DENGINE_DEBUG_ENTER;

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
    DENGINE_DEBUG_ENTER;

#ifdef DENGINE_ANDROID
    return dengineutils_android_iswindowrunning();
#else
    return window->running;
#endif
}

void* dengine_window_get_proc(const char* name)
{
    DENGINE_DEBUG_ENTER;

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
    DENGINE_DEBUG_ENTER;

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
    DENGINE_DEBUG_ENTER;

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
    DENGINE_DEBUG_ENTER;

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
    DENGINE_DEBUG_ENTER;

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
    #ifdef DENGINE_WIN_X11
    int h;
    dengine_window_get_dim(window, NULL, &h);
    char key;
    KeySym keysym;

    while(XPending(x_dpy))
    {
        //cannot read client messages 😑
//        polled = XCheckWindowEvent(x_dpy, window->x_win,
//                  window->x_swa.event_mask,
//                  &window->ev);
        XNextEvent(x_dpy, &window->ev);
        if(window->ev.type == ClientMessage && window->ev.xclient.data.l[0] == wm_delete)
        {
            window->running = 0;
        }

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
    while(window->running && GetMessageW(&window->win32_msg, window->win32_hwnd, 0, 0))
    {
        TranslateMessage(&window->win32_msg);
        DispatchMessageW(&window->win32_msg);
    }
    #elif defined(DENGINE_ANDROID)
    if(window && dengineutils_android_iswindowrunning())
    {
        int h;
        dengine_window_get_dim(window, NULL, &h);
        AndroidInput* andr_input = dengineutils_android_get_input();
        if(andr_input->pointer0.state == 0)
        {
            window->input.mse_x = andr_input->pointer0.x;
            window->input.mse_y = (float)h - andr_input->pointer0.y;
            if(window->input.msebtn[0] != -1)
                window->input.msebtn[0] = 1;
        }else
        {
            window->input.msebtn[0] = 0;
            window->input.mse_x = 0.0;
            window->input.mse_y = 0.0;
        }
    }
    #elif defined(DENGINE_WIN_WAYLAND)
    wl_display_dispatch_pending(wl_dpy);
    #endif
    return NULL;
}

int dengine_window_poll(DengineWindow* window)
{
    DENGINE_DEBUG_ENTER;

    int polled = 0;

#ifdef DENGINE_ANDROID
    dengineutils_android_pollevents();
#endif
    if(window != NULL && window->deref == 0){
        window->deref = 1;
        dengineutils_thread_condition_raise(&window->pollcond);
    }

#ifdef DENGINE_HAS_GTK3
   gtk_main_iteration_do(0);
#endif

   return polled;
}

int dengine_window_resize(DengineWindow* window, int width, int height)
{
    int ret = 0;
#ifdef DENGINE_WIN_X11
    ret = XResizeWindow(x_dpy, window->x_win, width, height);
#elif defined(DENGINE_WIN32)
    ret = MoveWindow(window->win32_hwnd, 0, 0, width, height, TRUE);
#elif defined(DENGINE_WIN_WAYLAND)
    wl_egl_window_resize(window->wl_egl_win, width, height, 0, 0);
#endif
    return ret;
}

void dengine_window_set_fullscreen(DengineWindow* window, int state)
{
    DENGINE_DEBUG_ENTER;

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
#elif defined (DENGINE_WIN_WAYLAND)
    if(state)
        xdg_toplevel_set_fullscreen(window->xdg_top, wl_out);
    else
        xdg_toplevel_unset_fullscreen(window->xdg_top);
#endif
}
int dengine_window_set_position(DengineWindow* window, int x, int y)
{
    DENGINE_DEBUG_ENTER;

    int set = 0;
#ifdef DENGINE_WIN_X11
    XWindowChanges changes;
    changes.x = x;
    changes.y = y;
    set = XConfigureWindow(x_dpy, window->x_win, CWX | CWY, &changes);
#elif defined(DENGINE_WIN32)
    set = SetWindowPos(window->win32_hwnd, window->win32_after, x, y,
                 window->width, window->height, SWP_FRAMECHANGED);
#elif defined(DENGINE_WIN_WAYLAND)
    /* TODO: WAIT FOR A WINDOW POSTION PROTOCOL
     * OR MESS WITH NASTY LIBWESTON
     *
     * THANKS ALOT WAYLAND! */
#endif
    return set;
}

WindowInput* dengine_window_get_input(DengineWindow* window)
{
    DENGINE_DEBUG_ENTER;

    return &window->input;
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
//        dengineutils_logging_log("WM_SIZE : %u", uMsg);
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        if(window && window->gl_load)
            dengine_viewport_set(0, 0, width, height);
        return 0;
    }

    case WM_PAINT:{
//        dengineutils_logging_log("WM_PAINT : %u", uMsg);
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_CLOSE:
    {
//        dengineutils_logging_log("WM_CLOSE : %u", uMsg);
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

#ifdef DENGINE_WIN_X11
int _dengine_window_x11err(Display* dpy, XErrorEvent* err)
{
    return 1;
}
#endif

#ifdef DENGINE_WIN_WAYLAND
static void registry_global(void* data, struct wl_registry* registry, uint32_t name, const char* ifc, uint32_t version)
{
//    dengineutils_logging_log("registry_global: ifc=%s, ver=%u", ifc, version);
    if(strcmp(ifc, wl_compositor_interface.name) == 0)
    {
        wl_comp = wl_registry_bind(registry, name,
                                   &wl_compositor_interface, version);
    }else if(strcmp(ifc, xdg_wm_base_interface.name) == 0)
    {
        wm_base = wl_registry_bind(registry, name,
                                 &xdg_wm_base_interface, version);
        xdg_wm_base_add_listener(wm_base, &wm_base_listener, NULL);
    }else if(strcmp(ifc, wl_output_interface.name) == 0)
    {
        wl_out = wl_registry_bind(registry, name,
                                  &wl_output_interface, version);
    }else if(strcmp(ifc, wl_seat_interface.name) == 0)
    {
        wl_seat = wl_registry_bind(registry, name,
                                   &wl_seat_interface, version);
        wl_seat_add_listener(wl_seat, &wl_seat_listener, NULL);
    }
}

static void registry_global_remove(void* data, struct wl_registry* registry, uint32_t name)
{
//    dengineutils_logging_log("registry_global_remove");
}

static void wm_base_ping(void* data, struct xdg_wm_base* wm, uint32_t serial)
{
//    dengineutils_logging_log("wm_base_ping: serial=%u", serial);
    xdg_wm_base_pong(wm, serial);
}

static void xdg_surface_configure(void* data, struct xdg_surface* xdg_sfc, uint32_t serial)
{
//    dengineutils_logging_log("xdg_surface_ack_configure: serial=%u", serial);
    xdg_surface_ack_configure(xdg_sfc, serial);
}

static void xdg_toplevel_configure(void* data, struct xdg_toplevel* toplvl, int w, int h, struct wl_array* states)
{
//    dengineutils_logging_log("xdg_toplevel_configure: w=%u, h=%u", w, h);
    if(w == 0 && h == 0)
        return;

    DengineWindow* window = data;
    wl_egl_window_resize(window->wl_egl_win, w, h, 0, 0);
    window->width = w;
    window->height = h;
    wl_surface_commit(window->wl_sfc);
}

static void xdg_toplevel_close(void* data, struct xdg_toplevel* toplvl)
{
//    dengineutils_logging_log("xdg_toplevel_close");
    DengineWindow* window = data;
    window->running = 0;
}
#if XDG_TOPLEVEL_CONFIGURE_BOUNDS_SINCE_VERSION >= 4
static void xdg_toplevel_configure_bounds(void *data,struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height)
{
//    dengineutils_logging_log("xdg_toplevel_configure_bounds: w=%u, h=%u", width, height);
}
#endif
#if XDG_TOPLEVEL_WM_CAPABILITIES_SINCE_VERSION >= 5
static void xdg_toplevel_wmcap(void *data, struct xdg_toplevel *xdg_toplevel, struct wl_array *capabilities)
{
//    dengineutils_logging_log("xdg_toplevel_wmcap");
}
#endif
static void wl_seat_name(void *data, struct wl_seat *wl_seat, const char *name)
{

}

static void wl_seat_caps(void* data, struct wl_seat* seat, uint32_t caps)
{
    if(caps & WL_SEAT_CAPABILITY_POINTER)
    {
        wl_ptr = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(wl_ptr, &wl_ptr_listener, NULL);
    }
    if(caps & WL_SEAT_CAPABILITY_KEYBOARD)
    {
        wl_kbd = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(wl_kbd, &wl_kbd_listener, NULL);
    }
}
/* currently focused window */
DengineWindow* focused_ptr = NULL;
DengineWindow* focused_kbd = NULL;
static void wl_pointer_enter(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t x, wl_fixed_t y)
{
    focused_ptr = wl_surface_get_user_data(surface);
}

static void wl_pointer_motion(void *data, struct wl_pointer *wl_pointer, uint32_t time, wl_fixed_t x, wl_fixed_t y)
{
    focused_ptr->input.mse_x = wl_fixed_to_double(x);
    focused_ptr->input.mse_y = focused_ptr->height - wl_fixed_to_double(y);
}

static void wl_pointer_frame(void* data, struct wl_pointer* pointer)
{

}
static void wl_pointer_button(void *data, struct wl_pointer *wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
    if(button == BTN_LEFT)
    {
        if(focused_ptr->input.msebtn[DENGINE_INPUT_MSEBTN_PRIMARY] != -1)
        {
            focused_ptr->input.msebtn[DENGINE_INPUT_MSEBTN_PRIMARY] = 1;
        }
        if(state == WL_POINTER_BUTTON_STATE_RELEASED)
            focused_ptr->input.msebtn[DENGINE_INPUT_MSEBTN_PRIMARY] = 0;

    }else if(button == BTN_RIGHT)
    {
        if(focused_ptr->input.msebtn[DENGINE_INPUT_MSEBTN_SECONDARY] != -1)
        {
            focused_ptr->input.msebtn[DENGINE_INPUT_MSEBTN_SECONDARY] = 1;
        }
        if(state == WL_POINTER_BUTTON_STATE_RELEASED)
            focused_ptr->input.msebtn[DENGINE_INPUT_MSEBTN_SECONDARY] = 0;
    }else if(button == BTN_MIDDLE)
    {
        if(focused_ptr->input.msebtn[DENGINE_INPUT_MSEBTN_MIDDLE] != -1)
        {
            focused_ptr->input.msebtn[DENGINE_INPUT_MSEBTN_MIDDLE] = 1;
        }
        if(state == WL_POINTER_BUTTON_STATE_RELEASED)
            focused_ptr->input.msebtn[DENGINE_INPUT_MSEBTN_MIDDLE] = 0;
    }
//    dengineutils_logging_log("btn:%u", button);
}
static void wl_pointer_leave(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface)
{
    focused_ptr = NULL;
}

static void wl_pointer_axis(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value)
{
//    dengineutils_logging_log("axis:%u value:%f", axis, wl_fixed_to_double(value));
    int value_int = wl_fixed_to_int(value);
    int value_norm = value_int / abs(value_int);
    if(axis == 0)
    {
        focused_ptr->input.msesrl_y = value_norm;
    }
}

static void wl_pointer_axis_stop(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis)
{

}

static void wl_pointer_axis_source(void *data, struct wl_pointer *wl_pointer, uint32_t axis_source)
{

}

static void wl_pointer_axis_discrete(void *data, struct wl_pointer *wl_pointer, uint32_t axis, int32_t discrete)
{

}

static void wl_keyboard_keymap(void *data, struct wl_keyboard *wl_keyboard, uint32_t format, int32_t fd, uint32_t size)
{
    xkb_state_unref(xkb_state);
    xkb_keymap_unref(xkb_keymap);

    char* charmap = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    xkb_keymap = xkb_keymap_new_from_string(xkb_ctx, charmap, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
    munmap(charmap, size);
    close(fd);

    xkb_state = xkb_state_new(xkb_keymap);
}
static void wl_keyboard_enter(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys)
{
    focused_kbd = wl_surface_get_user_data(surface);
}
static void wl_keyboard_leave(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface)
{
    focused_kbd = NULL;
}
static void wl_keyboard_mods(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group)
{
    xkb_state_update_mask(xkb_state,
                          mods_depressed, mods_latched, mods_locked,
                          0, 0, group);
}
static void wl_keyboard_key(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    char kv[2];
    xkb_state_key_get_utf8(xkb_state, key + 8, kv, sizeof(kv));
    char up = toupper(kv[0]);
    _dengine_window_processkey(&focused_kbd->input, up, !state);
//    dengineutils_logging_log("wl_keyboard_key: kv=%c, state=%u", kv[0], state);
}

static void wl_keyboard_repeatinfo (void *data, struct wl_keyboard *wl_keyboard, int32_t rate, int32_t delay)
{

}

#endif


