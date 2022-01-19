#ifndef OS_H
#define OS_H

#ifdef __cplusplus
extern "C" {
#endif

void dengineutils_os_init(int argc, char** argv);

int dengineutils_os_mkdir(const char* directory);

void dengineutils_os_dialog_messagebox(const char* title , const char* message, int error);

char* dengineutils_os_dialog_fileopen();

char* dengineutils_os_dialog_filesave();

#ifdef __cplusplus
}
#endif

#endif // OS_H
