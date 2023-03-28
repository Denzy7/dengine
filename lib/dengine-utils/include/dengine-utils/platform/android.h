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

    char keys[4];

    float accelarometer[3];
}AndroidInput;

#ifdef __cplusplus
extern "C" {
#endif

struct android_app* dengineutils_android_get_app();

void dengineutils_android_set_app(struct android_app* app);

void dengineutils_android_set_initfunc(DengineAndroidAppFunc func);

void dengineutils_android_set_terminatefunc(DengineAndroidAppFunc func);

int dengineutils_android_pollevents();

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
