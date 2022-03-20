#ifndef LOGGING_H
#define LOGGING_H

#ifdef __cplusplus
extern "C" {
#endif

void dengineutils_logging_set_filelogging(int value);

void dengineutils_logging_set_msgboxerror(int value);

void dengineutils_logging_set_consolecolor(char head);

void dengineutils_logging_log(const char* message, ...);

#ifdef __cplusplus
}
#endif

#endif // LOGGING_H


