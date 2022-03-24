#ifndef ANDROID_H
#define ANDROID_H

#include <android/native_app_glue/android_native_app_glue.h>

#include "dengine-utils/filesys.h" //f2m

typedef void (*DengineAndroidAppFunc)(struct android_app*);

#ifdef __cplusplus
extern "C" {
#endif

void dengine_android_set_app(struct android_app* app);

void dengine_android_set_initfunc(DengineAndroidAppFunc func);

void dengine_android_set_terminatefunc(DengineAndroidAppFunc func);

void dengine_android_pollevents();

char* dengine_android_getfilesdir();

char* dengine_android_getcachedir();

int dengine_android_asset2file2mem(File2Mem* f2m);

#ifdef __cplusplus
}
#endif

#endif // ANDROID_H
