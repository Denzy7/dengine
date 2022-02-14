#include "shader.h"
#include "loadgl.h" //shader

#include <stdio.h>  //printf
#include <stdlib.h> //malloc
#include <string.h> //strlen
#include "logging.h"//log

void dengine_shader_create(Shader* shader)
{
    shader->geometry_code= NULL;
}

void dengine_shader_destroy(Shader* shader)
{
    glDeleteShader(shader->fragment_id); DENGINE_CHECKGL;
    glDeleteShader(shader->vertex_id); DENGINE_CHECKGL;

#if defined(DENGINE_GL_GLAD) || defined(DENGINE_GL_GLES32)
    if(shader->geometry_code)
        glDeleteShader(shader->geometry_id); DENGINE_CHECKGL;
#endif

    glDeleteProgram(shader->program_id); DENGINE_CHECKGL;
}

int dengine_shader_compile_shader(const uint32_t shader, const char* code)
{
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
        }
#if defined(DENGINE_GL_GLAD) || defined(DENGINE_GL_GLES32)
        else if(type == GL_GEOMETRY_SHADER)
        {
            type_str = "GEOMETRY";
        }
#endif
        size_t line, column, zero;
        sscanf(info_log, "%zu:%zu(%zu)", &zero, &line, &column);

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
    shader->program_id = glCreateProgram(); DENGINE_CHECKGL;


    shader->vertex_id = glCreateShader(GL_VERTEX_SHADER); DENGINE_CHECKGL;
    shader->fragment_id = glCreateShader(GL_FRAGMENT_SHADER); DENGINE_CHECKGL;

    if(dengine_shader_compile_shader(shader->vertex_id, shader->vertex_code) &&
            dengine_shader_compile_shader(shader->fragment_id, shader->fragment_code))
    {
        glAttachShader(shader->program_id, shader->vertex_id); DENGINE_CHECKGL;
        glAttachShader(shader->program_id, shader->fragment_id); DENGINE_CHECKGL;

#if defined(DENGINE_GL_GLAD) || defined(DENGINE_GL_GLES32)
        if(shader->geometry_code)
        {
            shader->geometry_id =glCreateShader(GL_GEOMETRY_SHADER); DENGINE_CHECKGL;
            if(dengine_shader_compile_shader(shader->geometry_id, shader->geometry_code))
            {
                glAttachShader(shader->program_id, shader->geometry_id); DENGINE_CHECKGL;
            }
        }
#endif

        return dengine_shader_link(shader);
    }else
    {
        return 0;
    }
}

int dengine_shader_link(Shader* shader)
{
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

        dengineutils_logging_log("ERROR::SHADER::LINK::%s",log_length, info_log);

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


