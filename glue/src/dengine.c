#include "dengine/dengine.h"
#include "dengine.ini.h"
#include <string.h> //memset
static DengineInitOpts DENGINE_INIT_OPTS;
int DENGINE_HAS_GOT_INIT_OPTS = 0;
extern unsigned char LICENSE_md[];
extern unsigned char dengine_ini[];
extern unsigned int dengine_ini_ln;

typedef struct
{
    const char* key;
    DengineType type;
    void* address;
}DengineInitConfKey;

DengineInitConfKey confkeys[] =
{
    {"window_width", DENGINEUTILS_TYPE_INT32, &DENGINE_INIT_OPTS.window_width},
    {"window_height", DENGINEUTILS_TYPE_INT32, &DENGINE_INIT_OPTS.window_height},
    {"window_msaa", DENGINEUTILS_TYPE_INT32, &DENGINE_INIT_OPTS.window_msaa},
    {"window_swapinterval", DENGINEUTILS_TYPE_INT32, &DENGINE_INIT_OPTS.window_swapinterval},
    {"font_size", DENGINEUTILS_TYPE_FLOAT, &DENGINE_INIT_OPTS.font_size},
    {"font_bitmapsize", DENGINEUTILS_TYPE_INT32, &DENGINE_INIT_OPTS.font_bitmapsize},
    {"cache_shaders", DENGINEUTILS_TYPE_INT32, &DENGINE_INIT_OPTS.cache_shaders},
    {"gl_min", DENGINEUTILS_TYPE_INT32, &DENGINE_INIT_OPTS.gl_min},
    {"gl_max", DENGINEUTILS_TYPE_INT32, &DENGINE_INIT_OPTS.gl_max},
    {"gl_core", DENGINEUTILS_TYPE_INT32, &DENGINE_INIT_OPTS.gl_core},
    {"gui_subdata", DENGINEUTILS_TYPE_INT32, &DENGINE_INIT_OPTS.gui_subdata},
};

#ifdef DENGINE_ANDROID
void backbutton_func(struct android_app* app);
#endif


#ifdef DENGINE_ANDROID
int _dengine_backbutton_pressed = 0;
#endif

const char* dengine_get_license()
{
    return (const char*)LICENSE_md;
}

DengineInitOpts* dengine_init_get_opts()
{
    if(DENGINE_HAS_GOT_INIT_OPTS)
        return &DENGINE_INIT_OPTS;

    memset(&DENGINE_INIT_OPTS, 0, sizeof(DENGINE_HAS_GOT_INIT_OPTS));

    //ALLOCATE FILESYS DIRECTORIES
    dengineutils_filesys_init();

#ifdef DENGINE_ANDROID
    if(dengineutils_android_get_app() == NULL)
    {
        dengineutils_logging_log("ERROR::ENSURE, you have used dengineutils_android_set_app in your android_main before using using any dengine_init* function");
        return NULL;
    }
#endif
    DENGINE_INIT_OPTS.window_createnative = 1;
    DENGINE_INIT_OPTS.window_title = "Dengine!";

    DENGINE_INIT_OPTS.enable_backfaceculling = 1;
    DENGINE_INIT_OPTS.enable_depth = 1;

#ifdef DENGINE_ANDROID
    /*
     * in most cases don't need logthr.
     * just android (to redirect stdout to logcat)
     * and dengitor (to redirect stdout to logging widget)
     */
    DENGINE_INIT_OPTS.enable_logthread = 1;
#ifdef DENGINE_LIGHTING_SHADOW3D
        dengineutils_logging_log("WARNING::Shadow3D compiled and enabled on mobile, its highly recommened you recompile and disable to avoid a huge performance penalty");
#endif
    
#endif

    const size_t prtbf_sz = 2048;
    char* prtbf = (char*) malloc(prtbf_sz);

    snprintf(prtbf, prtbf_sz, "%s/dengine", dengineutils_filesys_get_filesdir());

    if(!dengineutils_os_direxist(prtbf))
        dengineutils_os_mkdir(prtbf);

    snprintf(prtbf, prtbf_sz, "%s/dengine/dengine.ini",
             dengineutils_filesys_get_filesdir());

    Conf* conf = dengineutils_confserialize_new(prtbf, '=');
    FILE* f_conf = fopen(prtbf, "rb");
    if(f_conf == NULL)
    {
        f_conf = fopen(prtbf, "wb");
        dengineutils_logging_log("TODO::create new init conf %s", prtbf);
        fwrite(dengine_ini, 1, dengine_ini_ln, f_conf);
        fclose(f_conf);
    }
    
    dengineutils_confserialize_load(conf, 1);
    dengineutils_logging_log("INFO::loaded init conf %s", prtbf);

    snprintf(prtbf, prtbf_sz, 
            "%s/dengine/dengine.%s.ini",
            dengineutils_filesys_get_filesdir(),
            DENGINE_INIT_CONF_VERSION);
    f_conf = fopen(prtbf, "rb");
    if(f_conf == NULL)
    {
        f_conf = fopen(prtbf, "wb");
        dengineutils_logging_log("TODO::create %s init conf %s", DENGINE_INIT_CONF_VERSION, prtbf);
        fwrite(dengine_ini, 1, dengine_ini_ln, f_conf);
        fclose(f_conf);
    }
    
    const char* currentconfver = dengineutils_confserialize_get_value("version", conf);
    if(currentconfver == NULL ||
            strcmp(currentconfver, DENGINE_INIT_CONF_VERSION) != 0)
    {
        dengineutils_logging_log("WARNING::your current version of dengine.ini may outdated! Consider migrating from %s.\n"
                "you could try running on unix (inside %s)\n\t"
                "diff -Naur dengine.ini dengine.%s.ini > dengine.%s.patch\n\t"
                "patch -p0 < dengine.%s.patch", 
                prtbf, prtbf,
                DENGINE_INIT_CONF_VERSION,
                DENGINE_INIT_CONF_VERSION,
                DENGINE_INIT_CONF_VERSION);
    }

    /*TODO: conf migration / versioning?? */
    /*else {*/
        /*fseek(f_conf, 0, SEEK_END);*/
        /*if(ftell(f_conf) != dengine_ini_len)*/
        /*{*/
            /*f_conf = fopen(prtbf, "wb");*/
            /*fwrite(dengine_ini, 1, dengine_ini_len, f_conf);*/
            /*fclose(f_conf);*/
        /*}*/
    /*}*/

    for(size_t i = 0; i < DENGINE_ARY_SZ(confkeys); i++)
    {
        const char* k = dengineutils_confserialize_get_value(confkeys[i].key, conf);
        if(k == NULL){
            dengineutils_logging_log("WARNING::conf doesn't have needed key [ %s ], please migrate or delete this invalid config file", confkeys[i].key);
            continue;
        }
        dengineutils_types_parse(confkeys[i].type, k, confkeys[i].address);
    }

    dengineutils_confserialize_free(conf);

    free(prtbf);

    DENGINE_HAS_GOT_INIT_OPTS = 1;

    return &DENGINE_INIT_OPTS;
}


int dengine_init()
{
    //DEBUGGING, INCASE OF SIGSEGV OR SIGABRT
    dengineutils_debug_init();

    DengineInitOpts* opts = &DENGINE_INIT_OPTS; 

    if(!DENGINE_HAS_GOT_INIT_OPTS)
        opts = dengine_init_get_opts();

    /* this fails if app isn't set on android since it returns NULL */
    if(opts == NULL)
        return 0;

    if(DENGINE_INIT_OPTS.window_msaa)
        dengine_window_request_MSAA(DENGINE_INIT_OPTS.window_msaa);

    if(DENGINE_INIT_OPTS.gl_max)
        dengine_window_request_GL(DENGINE_INIT_OPTS.gl_max,
                                  DENGINE_INIT_OPTS.gl_min,
                                  DENGINE_INIT_OPTS.gl_core);

    //All this to GL initialization
    if(DENGINE_INIT_OPTS.window_createnative)
    {
        if(!dengine_window_init())
        {
            dengineutils_logging_log("ERROR::Cannot init window");
            return 0;
        }
#ifdef DENGINE_ANDROID
        if(DENGINE_INIT_OPTS.android_handlebackbutton){
            dengineutils_android_handle_backbutton(1);
            dengineutils_android_set_backbuttonfunc(backbutton_func);
        }
#endif

        DENGINE_INIT_OPTS.window = dengine_window_create(DENGINE_INIT_OPTS.window_width, DENGINE_INIT_OPTS.window_height, DENGINE_INIT_OPTS.window_title, NULL);
        if(!DENGINE_INIT_OPTS.window)
        {
            dengineutils_logging_log("ERROR::Cannot create window");
            return 0;
        }

         if(!dengine_window_makecurrent(DENGINE_INIT_OPTS.window))
         {
             dengineutils_logging_log("ERROR::Cannot makecurrent window");
             return 0;
         }
         dengine_window_set_swapinterval(DENGINE_INIT_OPTS.window,
                                         DENGINE_INIT_OPTS.window_swapinterval);
         /*
          * set window to poll for input. not really necessary since
          * make current alreadt does this
          */
         dengine_input_set_input( dengine_window_get_input(DENGINE_INIT_OPTS.window));
    }

    if(DENGINE_INIT_OPTS.gl_loaddefault)
    {
        //use default glad loader
        if(!gladLoadGL())
        {
            dengineutils_logging_log("ERROR::Cannot gladLoadGL");
            return 0;
        }

    }else
    {
        //use window loader
        if(!dengine_window_loadgl(DENGINE_INIT_OPTS.window))
        {
            dengineutils_logging_log("ERROR::Cannot dengine_window_loadgl");
            return 0;
        }
    }

    //caching
    dengine_texture_set_texturecache(DENGINE_INIT_OPTS.cache_textures);
    dengine_shader_set_shadercache(DENGINE_INIT_OPTS.cache_shaders);
    denginegui_use_subdata(DENGINE_INIT_OPTS.gui_subdata);

    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    int samples;
    glGetIntegerv(GL_SAMPLES, &samples);
    const char* GL = (const char*)glGetString(GL_VERSION);
    const char* GLSL = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
    const char* VENDOR = (const char*)glGetString(GL_VENDOR);
    const char* RENDERDER = (const char*)glGetString(GL_RENDERER);
    char msaastr[9];
    if(samples)
        snprintf(msaastr, 9, "%dx MSAA", samples);
    else
        snprintf(msaastr, 9, "NO MSAA");

    dengineutils_logging_log("TODO::DENGINE : %s, BuildType: %s", DENGINE_VERSION, CMAKE_BUILD_TYPE);
    dengineutils_logging_log("INFO::GL : %s\nGLSL : %s\nVENDOR : %s\nRENDERDER : %s\n"
                             "VIEWPORT : %dx%d %s swapinterval(requested):%d",
                             GL, GLSL, VENDOR, RENDERDER,
                             viewport[2], viewport[3], msaastr, DENGINE_INIT_OPTS.window_swapinterval);

    //GUI. SET FONT TOO
    if(!denginegui_init())
        return 0;

    denginegui_set_font(NULL, DENGINE_INIT_OPTS.font_size, DENGINE_INIT_OPTS.font_bitmapsize);

    //SEED RNG. NOT MT-SAFE!(AFAIK)
    dengineutils_rng_set_seedwithtime();


    //INIT SCRIPTING
    denginescript_init();

    //depth testing ✅
    if(DENGINE_INIT_OPTS.enable_depth)
        glEnable(GL_DEPTH_TEST);

    //backface culling. save draw calls ✅
    if(DENGINE_INIT_OPTS.enable_backfaceculling)
        glEnable(GL_CULL_FACE);

    /*
     * should be one of the last thing to init so we get as much 
     * init info into stdout in case of crash
     */
    if(DENGINE_INIT_OPTS.enable_logthread)
        dengineutils_logging_init();

    return 1;
}

void dengine_terminate()
{
    denginegui_terminate();
    dengineutils_filesys_terminate();

    denginescript_terminate();
    if(DENGINE_INIT_OPTS.enable_logthread)
        dengineutils_logging_terminate();

    if(DENGINE_INIT_OPTS.window_createnative)
    {
        dengine_window_destroy(DENGINE_INIT_OPTS.window);
        dengine_window_terminate();
    }

    /* last to term to ensure trace dumped */
    dengineutils_debug_terminate();
}

int dengine_update()
{
    dengineutils_timer_update();
    if(DENGINE_INIT_OPTS.window_createnative){
        dengine_window_swapbuffers(DENGINE_INIT_OPTS.window);
        dengine_window_poll(DENGINE_INIT_OPTS.window);
    }
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#ifdef DENGINE_ANDROID
/* we need to still poll even when not in focus
 * so we can recreate egl context on resume
 */
    while(!dengineutils_android_get_activityfocused())
    {
        dengineutils_android_pollevents();
        if(!dengineutils_android_iswindowrunning())
            break;
    }
    if(_dengine_backbutton_pressed){
        _dengine_backbutton_pressed = 0;
        return 0;
    }
#endif
    if(DENGINE_INIT_OPTS.window_createnative)
        return dengine_window_isrunning(DENGINE_INIT_OPTS.window);
    else
        return 1;
}

int dengine_load_asset(const char* path, void** mem, size_t* length)
{
    File2Mem f2m;
    memset(&f2m, 0, sizeof(f2m));
#ifdef DENGINE_ANDROID
    f2m.file = path;
    dengineutils_android_asset2file2mem(&f2m);
#else
    char buf[2048];

    Stream assets_zip_stream;
    ZipRead assets_zip;
    CDFHR* cdfhr;
    uint32_t cdfhr_sz;
    /* hit or miss, i guess they never miss huh :( */
    static const char* possiblepaths[] = 
    {
        "../../../share/dengine-%s/assets.zip",
        "../../share/dengine-%s/assets.zip",
        "../share/dengine-%s/assets.zip",
        "share/dengine-%s/assets.zip",
        "assets.zip"
    };
    for(int i = 0; i < DENGINE_ARY_SZ(possiblepaths); i++)
    {
        snprintf(buf, sizeof(buf), possiblepaths[i], DENGINE_VERSION);
        FILE* ok = fopen(buf, "rb");
        if(ok != NULL)
        {
            fclose(ok);
            if(dengineutils_stream_new(buf, DENGINEUTILS_STREAM_TYPE_FILE, DENGINEUTILS_STREAM_MODE_READ, &assets_zip_stream))
            {
                dengineutils_zipread_load(&assets_zip_stream, &assets_zip);
                if(dengineutils_zipread_find_cdfhr(path, &cdfhr, &assets_zip))
                {
                    dengineutils_zipread_decompress_cdfhr_mem(&assets_zip_stream, cdfhr, &f2m.mem, &cdfhr_sz);
                    dengineutils_logging_log("TODO::load %s from assets.zip", path);
                    f2m.size = cdfhr_sz;
                }
            }

            dengineutils_zipread_free(&assets_zip);
            break;
        }
    }

    if(f2m.mem == NULL)
    {
        snprintf(buf, sizeof(buf),
                "%s/%s",
                dengineutils_filesys_get_assetsdir(), path);
        f2m.file = buf;
        dengineutils_filesys_file2mem_load(&f2m);

    }
#endif
    *mem = f2m.mem;
    if(length)
        *length = f2m.size;
    return 1;
}

void dengine_input_swinput_joystick(
        int x, int y, int dim, 
        int snap_lim,
        Texture* clamp, float* clamp_color_vec4,
        Texture* handle, float* handle_color_vec4, 
        float* outx, float* outy,
        SWInput_Joystick* joystick)
{
    float hwinput_x, hwinput_y;
    vec2 output;

    /* TODO: we use mouse for now. use touches too */
    joystick->isdown = dengine_input_get_mousebtn(DENGINE_INPUT_MSEBTN_PRIMARY);
    hwinput_x = dengine_input_get_mousepos_x();
    hwinput_y = dengine_input_get_mousepos_y();

    int joyx = x;
    int joyy = y;
    int joyhanddim = dim / 2;
    int joyhandx, joyhandy;

    int sqregionoffset = 
        snap_lim ?
        snap_lim : dim;

    /*TODO: doesn't consider the center of joystick. the user 
     * wont notice anyway (0_-), (-_-), (0_-)
     */
    int inregion = hwinput_x >= x - sqregionoffset && hwinput_x <= x + sqregionoffset &&
        hwinput_y >= y - sqregionoffset && hwinput_y <= y + sqregionoffset;

    if(snap_lim)
    {
        joyx = hwinput_x - (dim / 2.0f);
        joyy = hwinput_y - (dim / 2.0f);
        if(inregion && !joystick->isregion_ondown)
        {
            joystick->snap_x = joyx;;
            joystick->snap_y = joyy;
            joystick->isregion_ondown = 1;
        }

        if(joystick->isdown)
        {
            joyx = joystick->snap_x;
            joyy = joystick->snap_y;
        }
    }else {
        if(joystick->isdown && inregion)
            joystick->isregion_ondown = 1;
    }
    
    if(joystick->isdown && joystick->isregion_ondown){
        joyhandx = hwinput_x - (joyhanddim / 2.0f);
        joyhandy = hwinput_y - (joyhanddim / 2.0f);
        vec2 a = {joyhandx + (joyhanddim / 2.0f), joyhandy + (joyhanddim / 2.0f)}; 
        vec2 b = {joyx + (dim / 2.0f), joyy + (dim / 2.0f)}; 
        vec2 c;
        glm_vec2_sub(a, b, c);
        float mag = glm_vec2_norm(c);
        if(mag > dim / 2.0f)
        {
            glm_vec2_scale_as(c, dim / 2.0f, c);
        }
        joyhandx = joyx + (joyhanddim / 2.0f) + c[0];
        joyhandy = joyy + (joyhanddim / 2.0f) + c[1];
        glm_vec2_divs(c, dim / 2.0f, output);
;
    }else{
        joyx = x;
        joyy = y;
        joystick->isregion_ondown = 0;
        joyhandx = joyx + (dim / 2.0f) - (joyhanddim / 2.0f);
        joyhandy = joyy + (dim / 2.0f) - (joyhanddim / 2.0f);
        glm_vec2_zero(output);
    }

    *outx = output[0];
    *outy = output[1];

    denginegui_set_panel_discard(1);
    denginegui_panel(joyx, joyy, dim, dim, clamp, NULL, clamp_color_vec4);
    denginegui_panel(joyhandx, joyhandy, joyhanddim, joyhanddim, handle, NULL, handle_color_vec4);
    denginegui_set_panel_discard(0);
}

#ifdef DENGINE_ANDROID
void backbutton_func(struct android_app* app)
{
    _dengine_backbutton_pressed = 1;
}
#endif
