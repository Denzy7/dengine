#ifndef LOGGING_H
#define LOGGING_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*LoggingCallback)(const char*);

int dengineutils_logging_init();

void dengineutils_logging_terminate();

void dengineutils_logging_log(const char* message, ...);

void dengineutils_logging_addcallback(LoggingCallback callback);

void dengineutils_logging_set_filelogging(int value);

void dengineutils_logging_set_msgboxerror(int value);

int dengineutils_logging_get_msgboxerror();

void dengineutils_logging_set_consolecolor(char head);

const char* dengineutils_logging_get_logfile();
#ifdef __cplusplus
}
#endif

#endif // LOGGING_H


