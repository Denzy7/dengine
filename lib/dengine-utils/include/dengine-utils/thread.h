#ifndef DENGINEUTILS_THREAD_H
#define DENGINEUTILS_THREAD_H

#include "dengine_config.h"
#ifdef DENGINE_WIN32
#include <processthreadsapi.h>
#include <synchapi.h> /* CONDITION_VARIABLE */
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

typedef struct
{
#ifdef DENGINE_WIN32
    CRITICAL_SECTION critsec;
    CONDITION_VARIABLE condvar;
#else
    pthread_mutex_t mutex;
    pthread_cond_t cond;
#endif
}Condition;

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

int dengineutils_thread_condition_create(Condition* condition);

int dengineutils_thread_condition_destroy(Condition* condition);

int dengineutils_thread_condition_wait(Condition* condition, int* deref_and_set_to_one);

int dengineutils_thread_condition_raise(Condition* condition);

#ifdef __cplusplus
}
#endif
#endif // DENGINEUTILS_THREAD_H
