#include "shader.h"
#include "loadgl.h" //shader

#include <stdio.h>  //printf
#include <stdlib.h> //malloc
#include <string.h> //strlen
#include "logging.h"//log
#include "dengine-utils/filesys.h"//assetdir
#include "dengine-utils/debug.h"

#ifdef DENGINE_ANDROID
#include <dengine/android.h>
#endif
static
float default_shader_col[3] = {1.0, 0.0, 0.0};

static const char *stdshaderssrcfiles[][3]=
{
    {"standard.vert.glsl", "standard.frag.glsl"},
    {"default.vert.glsl", "default.frag.glsl"},
    {"shadow2d.vert.glsl", "shadow2d.frag.glsl"},
    {"shadow3d.vert.glsl", "shadow3d.frag.glsl", "shadow3d.geom.glsl"},
    {"gui.vert.glsl", "gui.frag.glsl"},
    {"debug/normals.vert.glsl", "debug/normals.frag.glsl"},
};

void dengine_shader_create(Shader* shader)
{
    shader->geometry_code= NULL;
}

void dengine_shader_destroy(Shader* shader)
{
    DENGINE_DEBUG_ENTER;

    glDeleteShader(shader->fragment_id); DENGINE_CHECKGL;
    glDeleteShader(shader->vertex_id); DENGINE_CHECKGL;

    if(shader->geometry_code)
        glDeleteShader(shader->geometry_id); DENGINE_CHECKGL;

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
    }

    return ok;

}

void dengine_shader_use(const Shader* shader)
{
    if(shader)
        glUseProgram(shader->program_id);
    else
        glUseProgram(0);

    DENGINE_CHECKGL;
}

void dengine_shader_set_int(const Shader* shader, const char* name, int value)
{
    dengine_shader_use(shader);
    int location = glGetUniformLocation(shader->program_id, name); DENGINE_CHECKGL;
    glUniform1i(location, value); DENGINE_CHECKGL;
}

void dengine_shader_set_mat4(const Shader* shader, const char* name, float* value)
{
    dengine_shader_use(shader);
    int location = glGetUniformLocation(shader->program_id, name); DENGINE_CHECKGL;
    glUniformMatrix4fv(location, 1, GL_FALSE, value); DENGINE_CHECKGL;
}

void dengine_shader_set_vec3(const Shader* shader, const char* name, float* value)
{
    dengine_shader_use(shader);
    int location = glGetUniformLocation(shader->program_id, name); DENGINE_CHECKGL;
    glUniform3fv(location, 1, value); DENGINE_CHECKGL;
}

void dengine_shader_set_vec4(const Shader* shader, const char* name, float* value)
{
    dengine_shader_use(shader);
    int location = glGetUniformLocation(shader->program_id, name); DENGINE_CHECKGL;
    glUniform4fv(location, 1, value); DENGINE_CHECKGL;
}

void dengine_shader_set_float(const Shader* shader, const char* name, float value)
{
    dengine_shader_use(shader);
    int location = glGetUniformLocation(shader->program_id, name); DENGINE_CHECKGL;
    glUniform1f(location, value); DENGINE_CHECKGL;
}

Shader* dengine_shader_new_shader_standard(StandardShader stdshader)
{
    DENGINE_DEBUG_ENTER;

    Shader* stdshdr = malloc(sizeof(Shader));
    memset(stdshdr,0,sizeof(Shader));

    dengine_shader_create(stdshdr);

    const int prtbuf_sz=2048;
    char* prtbuf = malloc(prtbuf_sz);;
    char *stdshdrsrc[3] =
    {
      NULL, NULL, NULL //Is this necessary?
    };

    File2Mem f2m;
    for (int i = 0; i < 3; i++) {
        const char* stdshdrsrcfile = stdshaderssrcfiles[stdshader][i];
        if(stdshdrsrcfile)
        {
            snprintf(prtbuf, prtbuf_sz, "%s/shaders/%s", dengineutils_filesys_get_assetsdir(), stdshdrsrcfile);
#ifdef DENGINE_ANDROID
            char* assetsshaders = strstr(prtbuf + 1, "shaders");
            f2m.file = assetsshaders;
            dengine_android_asset2file2mem(&f2m);
#else
            f2m.file = prtbuf;
            dengineutils_filesys_file2mem_load(&f2m);
#endif
            stdshdrsrc[i] = strdup(f2m.mem);
            dengineutils_filesys_file2mem_free(&f2m);
        }
    }

    stdshdr->vertex_code = stdshdrsrc[0];
    stdshdr->fragment_code = stdshdrsrc[1];
    stdshdr->geometry_code = stdshdrsrc[2];

    dengine_shader_setup(stdshdr);

    for (int i = 0; i < 2; i++) {
        char* stdshdrsrcdup = stdshdrsrc[i];
        if(stdshdrsrcdup)
        {
            free(stdshdrsrcdup);
        }
    }

    free(prtbuf);

    if(stdshader == DENGINE_SHADER_DEFAULT)
    {
        dengine_shader_set_vec3(stdshdr,"color", default_shader_col);
    }

    return stdshdr;
}
