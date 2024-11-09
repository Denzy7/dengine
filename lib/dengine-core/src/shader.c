#include "dengine/shader.h"
#include "dengine/loadgl.h" //shader

#include <stdio.h>  //printf
#include <stdlib.h> //malloc
#include <string.h> //strlen
#include "dengine-utils/logging.h"//log
#include "dengine-utils/filesys.h"//assetdir
#include "dengine-utils/debug.h"
#include "dengine-utils/os.h" //mkdir
#include "dengine_config.h" //cache_dir, ext, version
#ifdef DENGINE_ANDROID
#include <dengine-utils/platform/android.h>
#endif

static const float default_shader_col[3] = {1.0, 0.0, 0.0};

static GLenum binfmt = 0;
const Shader* current_shader = NULL;

static int shadercache = 1;

//dengine-rc output
extern char standard_vert_glsl[];
extern char standard_frag_glsl[];

extern char default_vert_glsl[];
extern char default_frag_glsl[];

extern char shadow2d_vert_glsl[];
extern char shadow2d_frag_glsl[];

extern char shadow3d_vert_glsl[];
extern char shadow3d_frag_glsl[];
extern char shadow3d_geom_glsl[];
extern char shadow3d_es_vert_glsl[];
extern char shadow3d_es_frag_glsl[];
extern char shadow3d_es_geom_glsl[];

extern char gui_vert_glsl[];
extern char gui_panel_frag_glsl[];
extern char gui_text_frag_glsl[];

extern char discard_frag_glsl[];

extern char debug_normals_vert_glsl[];
extern char debug_normals_frag_glsl[];

extern char skybox_vert_glsl[];
extern char skyboxcube_frag_glsl[];
extern char skybox2d_frag_glsl[];

static const char* const stdshaderssrcfiles[DENGINE_SHADER_COUNT][7]=
{
    //name(for program binary), vertex_shader, fragment_shader, geometry_shader, vert_es, frag_es, geom_es
    {"standard", standard_vert_glsl, standard_frag_glsl},
    {"default", default_vert_glsl ,default_frag_glsl},
    {"shadow2d", shadow2d_vert_glsl, shadow2d_frag_glsl},
    {"shadow3d", shadow3d_vert_glsl, shadow3d_frag_glsl, shadow3d_geom_glsl, shadow3d_es_vert_glsl, shadow3d_es_frag_glsl, shadow3d_es_geom_glsl},
    {"gui_text", gui_vert_glsl, gui_text_frag_glsl},
    {"gui_panel", gui_vert_glsl, gui_panel_frag_glsl},
    {"gui_discard", gui_vert_glsl, discard_frag_glsl},
    {"debug-normals", debug_normals_vert_glsl, debug_normals_frag_glsl},
    {"skyboxcube", skybox_vert_glsl, skyboxcube_frag_glsl},
    {"skybox2d", skybox_vert_glsl, skybox2d_frag_glsl},
};
typedef struct 
{
    Shader shader;
    int loaded;
}LoadedStdShdrs;
int loadedstdshdrs_init = 0;
LoadedStdShdrs loadedstdshdrs[DENGINE_SHADER_COUNT];

void _dengine_shader_set_binfmt();

void _dengine_shader_build_cachedir(char* ret, const size_t retlen);
void _dengine_shader_build_cachefile(char* ret, const size_t retlen, const char* cached_name);

int _dengine_shader_incache(const char* file);

/* filesys must be init! */
void _dengine_shader_build_cachefile(char* ret, const size_t retlen, const char* cached_name)
{
    char dir[2048];
    _dengine_shader_build_cachedir(dir, sizeof(dir));
    snprintf(ret, retlen, "%s/%s%s", dir, 
            cached_name,DENGINE_SHADER_CACHE_EXT);

    /* implicitly mkdir */
    if(!dengineutils_os_direxist(dir))
        dengineutils_os_mkdir(dir);
}

/* filesys must be init! */
void _dengine_shader_build_cachedir(char* ret, const size_t retlen)
{
    const char* GL = (const char*) glGetString(GL_VERSION);
    DENGINE_CHECKGL;
    snprintf(ret, retlen, "%s/%s/%s/%s", dengineutils_filesys_get_cachedir(),
                 DENGINE_SHADER_CACHE_DIR, DENGINE_VERSION, GL);
}

int _dengine_shader_incache(const char* cached_name)
{
    char prtbf[2048];
    FILE* test; 
    int ret = 0;
    if(dengineutils_filesys_isinit() && shadercache)
    {
        _dengine_shader_build_cachefile(prtbf, sizeof(prtbf), cached_name);
        test = fopen(prtbf, "rb");
        if(test){
            ret = 1;
            fclose(test);
        }
    }
    return ret;
}

void dengine_shader_create(Shader* shader)
{
    DENGINE_DEBUG_ENTER;

    shader->geometry_code= NULL;
    shader->cached_name = NULL;
}

void dengine_shader_destroy(Shader* shader)
{
    DENGINE_DEBUG_ENTER;

    glDeleteShader(shader->fragment_id); DENGINE_CHECKGL;
    glDeleteShader(shader->vertex_id); DENGINE_CHECKGL;

    if(shader->geometry_code)
        {glDeleteShader(shader->geometry_id); DENGINE_CHECKGL;}

    glDeleteProgram(shader->program_id); DENGINE_CHECKGL;
}

int dengine_shader_compile_shader(const uint32_t shader, const char* code)
{
    DENGINE_DEBUG_ENTER;

    glShaderSource(shader, 1, &code, NULL); DENGINE_CHECKGL;
    glCompileShader(shader); DENGINE_CHECKGL;
    int ok;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok); DENGINE_CHECKGL;
    if(!ok)
    {
        int log_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length); DENGINE_CHECKGL;
        char* info_log = malloc(log_length);
        glGetShaderInfoLog(shader, log_length, NULL, info_log); DENGINE_CHECKGL;

        int type;
        glGetShaderiv(shader, GL_SHADER_TYPE, &type); DENGINE_CHECKGL;
        const char* type_str = "VERTEX";

        if(type == GL_FRAGMENT_SHADER)
        {
            type_str = "FRAGMENT";
        }else if(type == GL_GEOMETRY_SHADER)
        {
            type_str = "GEOMETRY";
        }
        uint32_t line, column, zero;
        sscanf(info_log, "%u:%u(%u)", &zero, &line, &column);

        size_t findline = 0;
        char* stacktrace = NULL;
        for(size_t i = 0; i < strlen(code); i++)
        {
            if(code[i] == '\n')
            {
                findline++;
                if(findline == line - 1)
                {
                    /* +1 skip '\n' */
                    size_t sz = strlen(code + i + 1);

                    const char* after = strchr(code + i + 1, '\n');
                    if(after)
                    {
                        size_t sz_after = strlen(after);
                        size_t sz_stacktrace = sz - sz_after;
                        // +2 = null-term and newline
                        stacktrace = malloc(sz_stacktrace + column + 1);
                        memset(stacktrace, 0, sz_stacktrace + column + 1);
                        char arrows[column];
                        memset(arrows, '-', column);
                        arrows[column] = '\0';
                        arrows[column - 1] = '^';
                        sprintf(stacktrace, "%.*s\n%s", (int)sz_stacktrace, code + i + 1, arrows);
                    }
                }
            }

        }

        dengineutils_logging_log("ERROR::SHADER::COMPILE::%s::%s\nTRACE:\n%s", type_str, info_log, stacktrace ? stacktrace : "...");
        if(stacktrace)
            free(stacktrace);

        free(info_log);
    }

    return ok;
}

int dengine_shader_setup(Shader* shader)
{
    DENGINE_DEBUG_ENTER;

    /* WE'LL JUST GET A LINKER ERROR ANYWAY! */
/*
    //Dont compile an unsupported version
    int maj = 0,min = 0, ver = 0, shadver = 0;
    const char* glslv = (char *) glGetString(GL_SHADING_LANGUAGE_VERSION);
    DENGINE_CHECKGL;

    //We might get something like blah blah ES blah ES maj.min
    sscanf(strchr(glslv, '.') - 1,"%d.%d", &maj, &min);
    ver = maj*=100;
    ver+=min;

    if (shader->vertex_code) {
        const char* verstr = strchr(shader->vertex_code, '#');
        if(verstr)
        {
            char vers[10];
            sscanf(shader->fragment_code, "%s %d",vers,&shadver);

            if (shadver > ver) {
                dengineutils_logging_log("WARNING::SHADER::Not compiled. #version %d is unsupported. Supported is %d", shadver, ver);
                return 0;
            }
        }
    }
*/
    shader->hint = DENGINE_SHADER_SCENEQUEUER_SHADERHINT_OPAQUE;
    shader->program_id = glCreateProgram(); DENGINE_CHECKGL;

    if(!binfmt)
        _dengine_shader_set_binfmt();

    /* try load from cache */
    if(dengineutils_filesys_isinit() &&
            shadercache &&
            shader->cached_name &&
            _dengine_shader_incache(shader->cached_name) &&
            binfmt)
    {
        char prtbf[4096];
        const size_t prtbf_sz = sizeof(prtbf);
        int binload = 0;
        FILE* binfile;

        _dengine_shader_build_cachefile(prtbf, prtbf_sz, shader->cached_name);
        binfile = fopen(prtbf, "rb");
        if(binfile && binfmt)
        {
            File2Mem f2m;
            f2m.file = prtbf;
            dengineutils_filesys_file2mem_load(&f2m);
            if(dengine_shader_set_binary(shader, f2m.mem, (int)f2m.size))
            {
                binload = 1;
                dengineutils_logging_log("TODO::load binary %s, len:%d", shader->cached_name, (int)f2m.size);
            }
            dengineutils_filesys_file2mem_free(&f2m);
        }

        if(binload)
            return 1;
    }

    /*else compile source */
    shader->vertex_id = glCreateShader(GL_VERTEX_SHADER); DENGINE_CHECKGL;
    shader->fragment_id = glCreateShader(GL_FRAGMENT_SHADER); DENGINE_CHECKGL;

    if(dengine_shader_compile_shader(shader->vertex_id, shader->vertex_code) &&
            dengine_shader_compile_shader(shader->fragment_id, shader->fragment_code))
    {
        glAttachShader(shader->program_id, shader->vertex_id); DENGINE_CHECKGL;
        glAttachShader(shader->program_id, shader->fragment_id); DENGINE_CHECKGL;

        if(shader->geometry_code)
        {
            if(GLAD_GL_ES_VERSION_3_2 || GLAD_GL_VERSION_3_2)
            {
                shader->geometry_id =glCreateShader(GL_GEOMETRY_SHADER); DENGINE_CHECKGL;
                if(dengine_shader_compile_shader(shader->geometry_id, shader->geometry_code))
                {
                    glAttachShader(shader->program_id, shader->geometry_id); DENGINE_CHECKGL;
                }
            }
        }

        return dengine_shader_link(shader);
    }else
    {
        return 0;
    }
}

int dengine_shader_link(Shader* shader)
{
    DENGINE_DEBUG_ENTER;

    glLinkProgram(shader->program_id); DENGINE_CHECKGL;

    int ok;
    glGetProgramiv(shader->program_id, GL_LINK_STATUS, &ok); DENGINE_CHECKGL;

    shader->linking_succeeded = ok;

    if(!ok)
    {
        int log_length;
        glGetProgramiv(shader->program_id, GL_INFO_LOG_LENGTH, &log_length); DENGINE_CHECKGL;
        char* info_log = malloc(log_length);

        glGetProgramInfoLog(shader->program_id, log_length, NULL, info_log); DENGINE_CHECKGL;

        dengineutils_logging_log("ERROR::SHADER::LINK::%s", info_log);

        free(info_log);
    }else
    {
        if(dengineutils_filesys_isinit() && shadercache && shader->cached_name)
        {
            char prtbf[4096];
            const size_t prtbf_sz = 4096;
            FILE* binfile;
            int len = 0;
            void* bin;

            bin = dengine_shader_get_binary(shader, &len);
            _dengine_shader_build_cachefile(prtbf, prtbf_sz, shader->cached_name);
            binfile = fopen(prtbf, "wb");

            if(bin && binfile)
            {
                fwrite(bin, len, 1, binfile);
                fclose(binfile);
                free(bin);
                dengineutils_logging_log("TODO::save binary %s, len:%d", shader->cached_name, len);
            }
        }
    }

    return ok;

}

void dengine_shader_use(const Shader* shader)
{
    DENGINE_DEBUG_ENTER;
    current_shader = shader;
    if(shader)
        glUseProgram(shader->program_id);
    else
        glUseProgram(0);

    DENGINE_CHECKGL;
}

void dengine_shader_set_int(const Shader* shader, const char* name, const int value)
{
    DENGINE_DEBUG_ENTER;
    dengine_shader_use(shader);
    dengine_shader_current_set_int(name, value);
}

void dengine_shader_set_mat4(const Shader* shader, const char* name, const float* value)
{
    DENGINE_DEBUG_ENTER;
    dengine_shader_use(shader);
    dengine_shader_current_set_mat4(name, value);
}

void dengine_shader_set_vec3(const Shader* shader, const char* name, const float* value)
{
    DENGINE_DEBUG_ENTER;
    dengine_shader_use(shader);
    dengine_shader_current_set_vec3(name, value);
}

void dengine_shader_set_vec4(const Shader* shader, const char* name, const float* value)
{
    DENGINE_DEBUG_ENTER;
    dengine_shader_use(shader);
    dengine_shader_current_set_vec4(name, value);
}

void dengine_shader_set_float(const Shader* shader, const char* name, const float value)
{
    DENGINE_DEBUG_ENTER;
    dengine_shader_use(shader);
    dengine_shader_current_set_float(name, value);
}

void dengine_shader_current_set_mat4(const char* name, const float* value)
{
    DENGINE_DEBUG_ENTER;
    int location = glGetUniformLocation(current_shader->program_id, name); DENGINE_CHECKGL;
    glUniformMatrix4fv(location, 1, GL_FALSE, value); DENGINE_CHECKGL;
}

void dengine_shader_current_set_vec3(const char* name, const float* value)
{
    DENGINE_DEBUG_ENTER;
    int location = glGetUniformLocation(current_shader->program_id, name); DENGINE_CHECKGL;
    glUniform3fv(location, 1, value); DENGINE_CHECKGL;
}

void dengine_shader_current_set_vec4(const char* name, const float* value)
{
    DENGINE_DEBUG_ENTER;
    int location = glGetUniformLocation(current_shader->program_id, name); DENGINE_CHECKGL;
    glUniform4fv(location, 1, value); DENGINE_CHECKGL;
}

void dengine_shader_current_set_float(const char* name, const float value)
{
    DENGINE_DEBUG_ENTER;
    int location = glGetUniformLocation(current_shader->program_id, name); DENGINE_CHECKGL;
    glUniform1f(location, value); DENGINE_CHECKGL;
}

void dengine_shader_current_set_int(const char* name, const int value)
{
    DENGINE_DEBUG_ENTER;
    int location = glGetUniformLocation(current_shader->program_id, name); DENGINE_CHECKGL;
    glUniform1i(location, value); DENGINE_CHECKGL;
}

int dengine_shader_make_standard(StandardShader type, Shader* out)
{
    DENGINE_DEBUG_ENTER;
    memset(out, 0, sizeof(Shader));

    if(!loadedstdshdrs_init)
    {
        memset(loadedstdshdrs, 0, sizeof(loadedstdshdrs));
        loadedstdshdrs_init = 1;
    }

    Shader* shader = &loadedstdshdrs[type].shader;

    if(loadedstdshdrs[type].loaded)
    {
        dengineutils_logging_log("INFO::Give already loaded stdshdr %u", type);
        memcpy(out, &loadedstdshdrs[type].shader, sizeof(Shader));
        return 1;
    }

    dengine_shader_create(shader);
    shader->cached_name = stdshaderssrcfiles[type][0];

    const char *stdshdrsrc[3] =
    {
        NULL, NULL, NULL //Is this necessary? YES!, its contents are undefined unless set explicitly
    };
    int isgles = 0;
#ifdef DENGINE_GL_GLAD
    if(GLAD_GL_ES_VERSION_3_2)
        isgles = 1;
#endif

    /* set vert, frag, geom */
    for (int i = 0; i < 3; i++) {
        //+1 to skip name
        const char* stdshdrsrcfile = stdshaderssrcfiles[type][i + 1];
        if(isgles && stdshaderssrcfiles[type][i + 1 + 3] != NULL){
            stdshdrsrcfile = stdshaderssrcfiles[type][i + 1 + 3];
        }
        if(stdshdrsrcfile)
            stdshdrsrc[i] = stdshdrsrcfile;
    }

    shader->vertex_code = stdshdrsrc[0];
    shader->fragment_code = stdshdrsrc[1];
    shader->geometry_code = stdshdrsrc[2];

    if(dengineutils_filesys_isinit() && shadercache && _dengine_shader_incache(shader->cached_name))
    {
        dengine_shader_setup(shader);
        memcpy(out, shader, sizeof(Shader));
        loadedstdshdrs[type].loaded = 1;
        return 1;
    }

    int setup = dengine_shader_setup(shader);

    if(type == DENGINE_SHADER_DEFAULT)
        dengine_shader_set_vec3(shader,"color", default_shader_col);

    loadedstdshdrs[type].loaded = 1;
    memcpy(out, shader, sizeof(Shader));

    return setup;
}

int dengine_shader_set_binary(Shader* shader, void* binary, int length)
{
    DENGINE_DEBUG_ENTER;

    if(!binfmt)
        return 0;

    int ok = 0;

    if(glad_glProgramBinary)
    {
        glProgramBinary(shader->program_id, binfmt, binary, length); DENGINE_CHECKGL;
        ok = 1;
    }else if(glad_glProgramBinaryOES)
    {
        glProgramBinaryOES(shader->program_id, binfmt, binary, length); DENGINE_CHECKGL;
        ok = 1;
    }
    return ok;
}

void* dengine_shader_get_binary(Shader* shader, int* length)
{
    DENGINE_DEBUG_ENTER;

    void* bin = NULL;

    if(glad_glGetProgramBinary)
    {
        glGetProgramiv(shader->program_id, GL_PROGRAM_BINARY_LENGTH, length); DENGINE_CHECKGL;
        bin = malloc(*length);
        glGetProgramBinary(shader->program_id, *length, NULL, &binfmt, bin); DENGINE_CHECKGL;
    }else if(glad_glGetProgramBinaryOES)
    {
        glGetProgramiv(shader->program_id, GL_PROGRAM_BINARY_LENGTH_OES, length); DENGINE_CHECKGL;
        bin = malloc(*length);
        glGetProgramBinaryOES(shader->program_id, *length, NULL, &binfmt, bin); DENGINE_CHECKGL;
    }else
    {
        dengineutils_logging_log("WARNING::Could not find a get binary format function!");
    }

    if(bin != NULL)
        _dengine_shader_set_binfmt();

    return bin;
}

void _dengine_shader_set_binfmt()
{
    if(!shadercache || !dengineutils_filesys_isinit())
        return;

    char fmtfile[2048], dir[1024];
    _dengine_shader_build_cachedir(dir, sizeof(dir));
    snprintf(fmtfile, sizeof(fmtfile),
            "%s/shader_cache_format.bin", dir);
    FILE* f = fopen(fmtfile, "rb");
    if(f){
        fread(&binfmt, 1, sizeof(binfmt), f);
        fclose(f);
    }

    if(binfmt){
        f = fopen(fmtfile, "wb");
        fwrite(&binfmt, 1, sizeof(binfmt), f);
        fclose(f);
    }
}

void dengine_shader_set_shadercache(int state)
{
    DENGINE_DEBUG_ENTER;

    shadercache = state;
}

const uint32_t dengine_shader_sampler2target(const uint32_t sampler)
{
    DENGINE_DEBUG_ENTER;

    if(sampler == GL_SAMPLER_2D)
        return GL_TEXTURE_2D;
    else if(sampler == GL_SAMPLER_CUBE)
        return GL_TEXTURE_CUBE_MAP;
    else
        return 0;
}
