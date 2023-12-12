#include "dengine/dengine.h"
#include "dengine.ini.h"
#include <string.h> //memset
DengineInitOpts DENGINE_INIT_OPTS;
int DENGINE_HAS_GOT_INIT_OPTS = 0;
extern unsigned char LICENSE_md[];
extern unsigned char dengine_ini[];
extern unsigned int dengine_ini_ln;

typedef enum{
    DENGINE_INIT_CONF_TYPE_INT,
    /*DENGINE_INIT_CONF_TYPE_STRING,*/
    DENGINE_INIT_CONF_TYPE_FLOAT,

    DENGINE_INIT_CONF_TYPE_COUNT
}DengineInitConfType;

typedef struct
{
    const char* key;
    DengineInitConfType type;
    void* set;
}DengineInitConfKey;

DengineInitConfKey confkeys[] =
{
    {"window_width", DENGINE_INIT_CONF_TYPE_INT, NULL},
    {"window_height", DENGINE_INIT_CONF_TYPE_INT, NULL},
    {"window_msaa", DENGINE_INIT_CONF_TYPE_INT, NULL},
    {"window_swapinterval", DENGINE_INIT_CONF_TYPE_INT, NULL},
    {"font_size", DENGINE_INIT_CONF_TYPE_FLOAT, NULL},
    {"font_bitmapsize", DENGINE_INIT_CONF_TYPE_INT, NULL},
    {"cache_shaders", DENGINE_INIT_CONF_TYPE_INT, NULL},
    {"gl_min", DENGINE_INIT_CONF_TYPE_INT, NULL},
    {"gl_max", DENGINE_INIT_CONF_TYPE_INT, NULL},
    {"gl_core", DENGINE_INIT_CONF_TYPE_INT, NULL},
};

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
    //Can safely set files and cachedirs
    dengineutils_android_set_filesdir();
    dengineutils_android_set_cachedir();
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
#endif

    const size_t prtbf_sz = 2048;
    char* prtbf = (char*) malloc(prtbf_sz);

    snprintf(prtbf, prtbf_sz, "%s/dengine", dengineutils_filesys_get_filesdir());

    if(!dengineutils_os_direxist(prtbf))
        dengineutils_os_mkdir(prtbf);

    snprintf(prtbf, prtbf_sz, "%s/dengine/dengine.ini",
             dengineutils_filesys_get_filesdir());

    /* bind sets to keys */
    confkeys[0].set = &DENGINE_INIT_OPTS.window_width;
    confkeys[1].set = &DENGINE_INIT_OPTS.window_height;
    confkeys[2].set = &DENGINE_INIT_OPTS.window_msaa;
    confkeys[3].set = &DENGINE_INIT_OPTS.window_swapinterval;
    confkeys[4].set = &DENGINE_INIT_OPTS.font_size;
    confkeys[5].set = &DENGINE_INIT_OPTS.font_bitmapsize;
    confkeys[6].set = &DENGINE_INIT_OPTS.cache_shaders;
    confkeys[7].set = &DENGINE_INIT_OPTS.gl_min;
    confkeys[8].set = &DENGINE_INIT_OPTS.gl_max;
    confkeys[9].set = &DENGINE_INIT_OPTS.gl_core;

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
        if(k == NULL || confkeys[i].set == NULL)
            continue;

        if(confkeys[i].type == DENGINE_INIT_CONF_TYPE_INT)
            sscanf(k, "%d", (int*)confkeys[i].set);
        else if(confkeys[i].type == DENGINE_INIT_CONF_TYPE_FLOAT)
            sscanf(k, "%f", (float*)confkeys[i].set);
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

    if(!DENGINE_HAS_GOT_INIT_OPTS)
        DENGINE_INIT_OPTS = *dengine_init_get_opts();

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
        //Android creates a window already in init and sets it current
        DENGINE_INIT_OPTS.window = dengine_window_get_current();
#else
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
#endif
         dengine_window_set_swapinterval(DENGINE_INIT_OPTS.window,
                                         DENGINE_INIT_OPTS.window_swapinterval);
         //set window to poll for input
         dengine_input_set_window(DENGINE_INIT_OPTS.window);
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

    dengineutils_logging_log("TODO::DENGINE : %s", DENGINE_VERSION);
    dengineutils_logging_log("INFO::GL : %s\nGLSL : %s\nVENDOR : %s\nRENDERDER : %s\n"
                             "VIEWPORT : %dx%d %s",
                             GL, GLSL, VENDOR, RENDERDER,
                             viewport[2], viewport[3], msaastr);

    //GUI. SET FONT TOO
    if(!denginegui_init())
        return 0;

    denginegui_set_font(NULL, DENGINE_INIT_OPTS.font_size, DENGINE_INIT_OPTS.font_bitmapsize);

    //SEED RNG. NOT MT-SAFE!(AFAIK)
    dengineutils_rng_set_seedwithtime();

    //init logthread
    if(DENGINE_INIT_OPTS.enable_logthread)
        dengineutils_logging_init();

    //INIT SCRIPTING
    denginescript_init();

    //depth testing ✅
    if(DENGINE_INIT_OPTS.enable_depth)
        glEnable(GL_DEPTH_TEST);

    //backface culling. save draw calls ✅
    if(DENGINE_INIT_OPTS.enable_backfaceculling)
        glEnable(GL_CULL_FACE);

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
    if(DENGINE_INIT_OPTS.window_createnative)
        return dengine_window_isrunning(DENGINE_INIT_OPTS.window);
    else
        return 1;
}

int dengine_load_asset(const char* path, void** mem, size_t* length)
{
    File2Mem f2m;
#ifdef DENGINE_ANDROID
    f2m.file = path;
    dengineutils_android_asset2file2mem(&f2m);
#else
    char buf[2048];
    snprintf(buf, sizeof(buf),
            "%s/%s",
            dengineutils_filesys_get_assetsdir(), path);
    f2m.file = buf;
    dengineutils_filesys_file2mem_load(&f2m);
#endif
    *mem = f2m.mem;
    if(length)
        *length = f2m.size;
    return 1;
}
