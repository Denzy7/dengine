#include "logging.h"//log

#include "loadgl.h" //glGetError (incl. checkgl.h
void dengine_checkgl(const char* file, const int line)
{
    GLenum error = glGetError();
    if(error)
    {
        const char* type_str = "GL_INVALID_ENUM\nAn unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag.";
        if (error == GL_INVALID_VALUE)
            type_str = "GL_INVALID_VALUE\nA numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag.";
        else if (error == GL_INVALID_OPERATION)
            type_str = "GL_INVALID_OPERATION\nThe specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag.";
        else if (error == GL_INVALID_FRAMEBUFFER_OPERATION)
            type_str = "GL_INVALID_FRAMEBUFFER_OPERATION\nThe framebuffer object is not complete. The offending command is ignored and has no other side effect than to set the error flag.";
        else if (error == GL_OUT_OF_MEMORY)
            type_str = "GL_OUT_OF_MEMORY\nThere is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";
        else if (error == GL_STACK_UNDERFLOW)
            type_str = "GL_STACK_UNDERFLOW\nAn attempt has been made to perform an operation that would cause an internal stack to underflow.";
        else if (error == GL_STACK_OVERFLOW)
            type_str = "GL_STACK_OVERFLOW\nAn attempt has been made to perform an operation that would cause an internal stack to overflow.";

        dengineutils_logging_log("ERROR::GL::FILE::%s::LINE::%d\n%s", file, line, type_str);
    }
}
