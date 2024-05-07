#ifndef ANDROID_H
#define ANDROID_H

#include <android/native_app_glue/android_native_app_glue.h>

#include "dengine-utils/filesys.h" //f2m

typedef void (*DengineAndroidAppFunc)(struct android_app*);

typedef struct
{
    float x, y;
    int state;
}AndroidPointer;

typedef struct
{
    AndroidPointer pointer0;
    AndroidPointer pointers[10];
    size_t pointer_count;

    char keys[4];

    float accelarometer[3];
}AndroidInput;

#ifdef __cplusplus
extern "C" {
#endif


typedef enum{
    DENGINEUTILS_ANDROID_APPFUNC_INITWINDOW,
    DENGINEUTILS_ANDROID_APPFUNC_TERMINATE,
    DENGINEUTILS_ANDROID_APPFUNC_RESUME,
    DENGINEUTILS_ANDROID_APPFUNC_PAUSE,

    DENGINEUTILS_ANDROID_APPFUNC_COUNT
}DengineAndroidAppFuncType;

struct android_app* dengineutils_android_get_app();

void dengineutils_android_set_app(struct android_app* app);

void dengineutils_android_set_appfunc(DengineAndroidAppFunc func, DengineAndroidAppFuncType type);

void dengineutils_android_set_initfunc(DengineAndroidAppFunc func);

void dengineutils_android_set_terminatefunc(DengineAndroidAppFunc func);

int dengineutils_android_pollevents();

int dengineutils_android_waitevents();

void dengineutils_android_set_filesdir();

void dengineutils_android_set_cachedir();

int dengineutils_android_asset2file2mem(File2Mem* f2m);

ANativeWindow* dengineutils_android_get_window();

AAssetManager* dengineutils_android_get_assetmgr();

int dengineutils_android_iswindowrunning();

AndroidInput* dengineutils_android_get_input();
#ifdef __cplusplus
}
#endif

#endif // ANDROID_H
