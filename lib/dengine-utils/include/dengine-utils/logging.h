#ifndef LOGGING_H
#define LOGGING_H

typedef enum
{
    DENGINE_LOGGING_COLOR_RED,
    DENGINE_LOGGING_COLOR_GREEN,
    DENGINE_LOGGING_COLOR_BLUE,

    DENGINE_LOGGING_COLOR_CYAN,
    DENGINE_LOGGING_COLOR_YELLOW,
    DENGINE_LOGGING_COLOR_MAGENTA,

    DENGINE_LOGGING_COLOR_WHITE,
    DENGINE_LOGGING_COLOR_RESET,
}ConsoleColor;

typedef void (*LoggingCallback)(const char* logbuf,const char* tripbuf);

#ifdef __cplusplus
extern "C" {
#endif

int dengineutils_logging_init();

void dengineutils_logging_terminate();

void dengineutils_logging_log(const char* message, ...);

void dengineutils_logging_addcallback(LoggingCallback callback);

void dengineutils_logging_set_filelogging(int value);

void dengineutils_logging_set_msgboxerror(int value);

int dengineutils_logging_get_msgboxerror();

void dengineutils_logging_set_consolecolor(ConsoleColor colour);

const char* dengineutils_logging_get_logfile();
#ifdef __cplusplus
}
#endif

#endif // LOGGING_H


