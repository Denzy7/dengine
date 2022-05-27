#ifndef DENGINEUTILS_THREAD_H
#define DENGINEUTILS_THREAD_H

#include "dengine_config.h"
#ifdef DENGINE_WIN32
#include <processthreadsapi.h>
#else
#include <pthread.h>
#endif
typedef void*(*threadstart)(void* arg);

typedef struct
{
#ifdef DENGINE_WIN32
    HANDLE hThread;
    DWORD dThread;
#else
    pthread_t thr;
#endif
}Thread;

typedef struct
{
#ifdef DENGINE_WIN32
    HANDLE hMutex;
#else
    pthread_mutex_t mutex;
#endif
}Mutex;

#ifdef __cplusplus
extern "C" {
#endif

int dengineutils_thread_create(threadstart start, void* arg, Thread* thread);

int dengineutils_thread_wait(Thread* thread);

void dengineutils_thread_exit(Thread* thread);

int dengineutils_thread_mutex_create(Mutex* mutex);

int dengineutils_thread_mutex_destroy(Mutex* mutex);

int dengineutils_thread_mutex_lock(Mutex* mutex);

int dengineutils_thread_mutex_unlock(Mutex* mutex);

#ifdef __cplusplus
}
#endif
#endif // DENGINEUTILS_THREAD_H
