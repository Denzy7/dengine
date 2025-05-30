#ifndef DENGINEUTILS_THREAD_H
#define DENGINEUTILS_THREAD_H

#include "dengine_config.h"
#ifdef DENGINE_WIN32
#include <processthreadsapi.h>
#include <synchapi.h> /* CONDITION_VARIABLE */
#else

#ifdef DENGINE_HAS_PTHREAD_TYPES_H
#include <bits/pthread_types.h>
#elif defined(DENGINE_HAS_PTHREADTYPES_H)
#include <bits/pthreadtypes.h>
#else /* pollutes namespace */
#include <pthread.h>
#endif

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
    int oned;
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

/* only for pthread on unix! */
int dengineutils_thread_set_name(Thread* thread, const char* name);

int dengineutils_thread_wait(Thread* thread);

void dengineutils_thread_exit(Thread* thread);

int dengineutils_thread_mutex_create(Mutex* mutex);

int dengineutils_thread_mutex_destroy(Mutex* mutex);

int dengineutils_thread_mutex_lock(Mutex* mutex);

int dengineutils_thread_mutex_unlock(Mutex* mutex);

int dengineutils_thread_condition_create(Condition* condition);

int dengineutils_thread_condition_destroy(Condition* condition);

/* deref_and_set_to_one prevents spurious wake up from 
 * the kernel to unexpectedly resume the thread.
 * set it to one just after or before calling thread_condition_raise 
 * to tell the thread to actually resume. you can think of it
 * as deref is the actual variable the thread is waiting for :)
 *
 * Condition cond;
 * int deref;
 * void* thr(void* arg)
 * {
 * ...
 *  condition_wait(&cond, &deref);
 * ... continue thread normally
 * }
 * void func_with_data_needed_by_thread(){
 * ...
 * deref = 1;
 * condition_raise(&cond);
 * ...
 * }
 *
 */
int dengineutils_thread_condition_wait(Condition* condition, int* deref_and_set_to_one);

/* same as condtion_wait, but uses the oned in
 * the Condition struct
 */
int dengineutils_thread_condition_wait_self(Condition* condition);

int dengineutils_thread_condition_raise(Condition* condition);

#ifdef __cplusplus
}
#endif
#endif // DENGINEUTILS_THREAD_H
