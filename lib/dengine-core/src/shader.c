#include "dengine/shader.h"
#include "dengine/loadgl.h" //shader

#include <stdio.h>  //printf
#include <stdlib.h> //malloc
#include <string.h> //strlen
#include "dengine-utils/logging.h"//log
#include "dengine-utils/filesys.h"//assetdir
#include "dengine-utils/debug.h"
#include "dengine-utils/str.h" //str_ndup
#include "dengine-utils/os.h" //mkdir
#include "dengine_config.h" //cache_dir, ext, version
#ifdef DENGINE_ANDROID
#include <dengine-utils/platform/android.h>
#endif

static const float default_shader_col[3] = {1.0, 0.0, 0.0};

static GLenum binfmt = 0;
const Shader* current = NULL;

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

extern char gui_vert_glsl[];
extern char gui_frag_glsl[];

extern char debug_normals_vert_glsl[];
extern char debug_normals_frag_glsl[];

extern char skybox_vert_glsl[];
extern char skyboxcube_frag_glsl[];
extern char skybox2d_frag_glsl[];

static const char *stdshaderssrcfiles[][4]=
{
    //name(for program binary), vertex_shader, fragment_shader, geometry_shader
    {"standard", standard_vert_glsl, standard_frag_glsl},
    {"default", default_vert_glsl ,default_frag_glsl},
    {"shadow2d", shadow2d_vert_glsl, shadow2d_frag_glsl},
    {"shadow3d", shadow3d_vert_glsl, shadow3d_frag_glsl, shadow3d_geom_glsl},
    {"gui", gui_vert_glsl, gui_frag_glsl},
    {"debug-normals", debug_normals_vert_glsl, debug_normals_frag_glsl},
    {"skyboxcube", skybox_vert_glsl, skyboxcube_frag_glsl},
    {"skybox2d", skybox_vert_glsl, skybox2d_frag_glsl},
};

void _dengine_shader_set_binfmt();

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

    shader->program_id = glCreateProgram(); DENGINE_CHECKGL;

    if(dengineutils_filesys_isinit() && shadercache)
    {
        const size_t prtbf_sz = 4096;
        char* prtbf = malloc(prtbf_sz);
        int binload = 0;

        const char* GL = (const char*) glGetString(GL_VERSION);
        DENGINE_CHECKGL;
        snprintf(prtbf, prtbf_sz, "%s/%s/%s/%s", dengineutils_filesys_get_cachedir(),
                 DENGINE_SHADER_CACHE_DIR, DENGINE_VERSION, GL);

        if(!dengineutils_os_direxist(prtbf))
            dengineutils_os_mkdir(prtbf);


        if(shader->cached_name)
        {
            snprintf(prtbf, prtbf_sz, "%s/%s/%s/%s/%s%s", dengineutils_filesys_get_cachedir(),
                     DENGINE_SHADER_CACHE_DIR, DENGINE_VERSION, GL,
                     shader->cached_name, DENGINE_SHADER_CACHE_EXT);

            if(fopen(prtbf, "rb"))
            {
                File2Mem f2m;
                f2m.file = prtbf;

                if(!binfmt)
                    _dengine_shader_set_binfmt();

                if(binfmt)
                {
                    dengineutils_filesys_file2mem_load(&f2m);
                    if(dengine_shader_set_binary(shader, f2m.mem, (int)f2m.size))
                    {
                        binload = 1;
                        dengineutils_logging_log("TODO::load binary %s, %d,%u", shader->cached_name, (int)f2m.size, binfmt);
                    }
                    dengineutils_filesys_file2mem_free(&f2m);
                }
            }
        }

        free(prtbf);

        if(binload)
            return 1;
    }

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
        if(dengineutils_filesys_isinit() && shadercache)
        {
            const size_t prtbf_sz = 4096;
            char* prtbf = malloc(prtbf_sz);
            const char* GL = (const char*) glGetString(GL_VERSION);
            DENGINE_CHECKGL;
            snprintf(prtbf, prtbf_sz, "%s/%s/%s/%s/%s%s", dengineutils_filesys_get_cachedir(),
                     DENGINE_SHADER_CACHE_DIR, DENGINE_VERSION, GL,
                     shader->cached_name, DENGINE_SHADER_CACHE_EXT);
            if(!fopen(prtbf, "rb") && shader->cached_name)
            {
                //fopen and save binary here
                if(!binfmt)
                    _dengine_shader_set_binfmt();

                if(binfmt)
                {
                    FILE* f_bin = fopen(prtbf, "wb");
                    int len = 0;
                    void* bin = dengine_shader_get_binary(shader, &len);
                    if(f_bin)
                    {
                        fwrite(bin, len, 1, f_bin);
                        fclose(f_bin);
                        free(bin);
                    }
                }
                dengineutils_logging_log("TODO::save binary %s", shader->cached_name);
            }

            free(prtbf);
        }

    }

    return ok;

}

void dengine_shader_use(const Shader* shader)
{
    DENGINE_DEBUG_ENTER;
    current = shader;
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
    int location = glGetUniformLocation(current->program_id, name); DENGINE_CHECKGL;
    glUniformMatrix4fv(location, 1, GL_FALSE, value); DENGINE_CHECKGL;
}

void dengine_shader_current_set_vec3(const char* name, const float* value)
{
    DENGINE_DEBUG_ENTER;
    int location = glGetUniformLocation(current->program_id, name); DENGINE_CHECKGL;
    glUniform3fv(location, 1, value); DENGINE_CHECKGL;
}

void dengine_shader_current_set_vec4(const char* name, const float* value)
{
    DENGINE_DEBUG_ENTER;
    int location = glGetUniformLocation(current->program_id, name); DENGINE_CHECKGL;
    glUniform4fv(location, 1, value); DENGINE_CHECKGL;
}

void dengine_shader_current_set_float(const char* name, const float value)
{
    DENGINE_DEBUG_ENTER;
    int location = glGetUniformLocation(current->program_id, name); DENGINE_CHECKGL;
    glUniform1f(location, value); DENGINE_CHECKGL;
}

void dengine_shader_current_set_int(const char* name, const int value)
{
    DENGINE_DEBUG_ENTER;
    int location = glGetUniformLocation(current->program_id, name); DENGINE_CHECKGL;
    glUniform1i(location, value); DENGINE_CHECKGL;
}

int dengine_shader_make_standard(StandardShader stdshader, Shader* stdshdr)
{
    DENGINE_DEBUG_ENTER;

    memset(stdshdr,0,sizeof(Shader));

    dengine_shader_create(stdshdr);

    const int prtbuf_sz=2048;
    char* prtbuf = malloc(prtbuf_sz);

    //get cache name
    char* cached = strdup(stdshaderssrcfiles[stdshader][0]);
    stdshdr->cached_name = cached;

    if(dengineutils_filesys_isinit() && shadercache)
    {
        int bin_success = 0;
        const char* GL = (const char*) glGetString(GL_VERSION);
        DENGINE_CHECKGL;
        snprintf(prtbuf, prtbuf_sz, "%s/%s/%s/%s/%s%s", dengineutils_filesys_get_cachedir(),
                 DENGINE_SHADER_CACHE_DIR, DENGINE_VERSION, GL,
                 stdshdr->cached_name, DENGINE_SHADER_CACHE_EXT);

        if(fopen(prtbuf, "rb"))
        {
            bin_success = dengine_shader_setup(stdshdr);
        }

        if(bin_success)
        {
            free(cached);
            free(prtbuf);
            return 1;
        }
    }

    char *stdshdrsrc[3] =
    {
      NULL, NULL, NULL //Is this necessary?
    };

    for (int i = 0; i < 3; i++) {
        //+1 to skip name
        const char* stdshdrsrcfile = stdshaderssrcfiles[stdshader][i + 1];
        if(stdshdrsrcfile)
            stdshdrsrc[i] = strdup(stdshdrsrcfile);
    }

    stdshdr->vertex_code = stdshdrsrc[0];
    stdshdr->fragment_code = stdshdrsrc[1];
    stdshdr->geometry_code = stdshdrsrc[2];

    int setup = dengine_shader_setup(stdshdr);

    for (int i = 0; i < 2; i++) {
        char* stdshdrsrcdup = stdshdrsrc[i];
        if(stdshdrsrcdup)
        {
            free(stdshdrsrcdup);
        }
    }
    free(prtbuf);
    free(cached);

    if(stdshader == DENGINE_SHADER_DEFAULT)
    {
        dengine_shader_set_vec3(stdshdr,"color", default_shader_col);
    }

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
        dengineutils_logging_log("WARNING::Could not find a binary format. Shader cache disabled");
    }

    return bin;
}

void _dengine_shader_set_binfmt()
{
    DENGINE_DEBUG_ENTER;

    Shader* binfmtshdr = calloc(1, sizeof (Shader));

    binfmtshdr->vertex_code = "void main(){gl_Position = vec4(1.0);}";
    binfmtshdr->fragment_code = "void main(){gl_FragColor = vec4(1.0);}";

    dengine_shader_setup(binfmtshdr);

    int len = 0;
    void* bin = dengine_shader_get_binary(binfmtshdr, &len);

    dengine_shader_destroy(binfmtshdr);

    if(bin)
        free(bin);
    free(binfmtshdr);
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
