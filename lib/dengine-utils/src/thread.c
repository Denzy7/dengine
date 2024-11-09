#include "dengine_config.h"
#ifdef DENGINE_WIN32
/*
 * include here windows since it breaks everything if
 * its in thread.h
 */
#include <windows.h>
#else
#define _GNU_SOURCE
#include <pthread.h>
#endif
#include "dengine-utils/thread.h"

int dengineutils_thread_set_name(Thread* thread, const char* name)
{
    int ret = 0;
#if defined (DENGINE_HAS_PTHREAD_SETNAME_NP) &&  defined (DENGINE_LINUX)
    ret = pthread_setname_np(thread->thr, name);
#endif
    return ret;
}

int dengineutils_thread_create(threadstart start, void* arg, Thread* thread)
{
    int threadok = 0;
#ifdef DENGINE_WIN32
    thread->hThread = CreateThread(NULL,
                                   0,
                                   (LPTHREAD_START_ROUTINE)start,
                                   arg,
                                   0,
                                   &thread->dThread);
    if(thread->hThread != NULL)
        threadok = 1;
#else
    threadok = pthread_create(&thread->thr, NULL, start, arg);
#endif

    return threadok;
}

int dengineutils_thread_wait(Thread* thread)
{
#ifdef DENGINE_WIN32
    return WaitForSingleObject(thread->hThread, INFINITE);
#else
    return pthread_join(thread->thr, NULL);
#endif
}

void dengineutils_thread_exit(Thread* thread)
{
#ifdef DENGINE_WIN32
    CloseHandle(thread->hThread);
#else

#ifdef DENGINE_HAS_PTHREAD_CANCEL
    pthread_cancel(thread->thr);
#endif

#endif
}

int dengineutils_thread_mutex_create(Mutex* mutex)
{
#ifdef DENGINE_WIN32
    mutex->hMutex = CreateMutex(NULL, FALSE, NULL);
    if(mutex->hMutex == NULL)
        return 0;
    else
        return 1;
#else
    return pthread_mutex_init(&mutex->mutex, NULL);
#endif
}

int dengineutils_thread_mutex_destroy(Mutex* mutex)
{
#ifdef DENGINE_WIN32
    return CloseHandle(mutex->hMutex);
#else
    return pthread_mutex_destroy(&mutex->mutex);
#endif
}

int dengineutils_thread_mutex_lock(Mutex* mutex)
{
#ifdef DENGINE_WIN32
    return WaitForSingleObject(mutex->hMutex, INFINITE);
#else
    return pthread_mutex_lock(&mutex->mutex);
#endif
}

int dengineutils_thread_mutex_unlock(Mutex* mutex)
{
#ifdef DENGINE_WIN32
    return ReleaseMutex(mutex->hMutex);
#else
    return pthread_mutex_unlock(&mutex->mutex);
#endif
}

int dengineutils_thread_condition_create(Condition* condition)
{
    int ret = 0;
#ifdef DENGINE_WIN32
    InitializeCriticalSection(&condition->critsec);
    InitializeConditionVariable(&condition->condvar);
#else
    pthread_cond_init(&condition->cond, NULL);
    pthread_mutex_init(&condition->mutex, NULL);
#endif
    return ret;
}

int dengineutils_thread_condition_destroy(Condition* condition)
{
    int ret = 0;
#ifdef DENGINE_WIN32
    DeleteCriticalSection(&condition->critsec);
    ret = 1;
#else
    ret = pthread_cond_destroy(&condition->cond) &&
            pthread_mutex_destroy(&condition->mutex);
#endif
    return ret;
}

int dengineutils_thread_condition_wait(Condition* condition, int* deref_and_set_to_one)
{
    /* LOCK MUTEX / ENTER CRITICAL SECTION */
#ifdef DENGINE_WIN32
    EnterCriticalSection(&condition->critsec);
#else
    pthread_mutex_lock(&condition->mutex);
#endif
    while(*deref_and_set_to_one == 0){
        /* SLEEP / WAIT */
#ifdef DENGINE_WIN32
        SleepConditionVariableCS(&condition->condvar,
                &condition->critsec,
                INFINITE);
#else
        pthread_cond_wait(&condition->cond, &condition->mutex);
#endif
    }

    /* UNLOCK MUTEX / LEAVE CS */
#ifdef DENGINE_WIN32
    LeaveCriticalSection(&condition->critsec);
#else
    pthread_mutex_unlock(&condition->mutex);
#endif

    return 1;
}

int dengineutils_thread_condition_wait_self(Condition* condition)
{
    return dengineutils_thread_condition_wait(condition, &condition->oned);
}
int dengineutils_thread_condition_raise(Condition* condition)
{
#ifdef DENGINE_WIN32
    WakeAllConditionVariable(&condition->condvar);
#else
    pthread_mutex_lock(&condition->mutex);
    pthread_cond_signal(&condition->cond);
    pthread_mutex_unlock(&condition->mutex);
#endif
    return 1;
}
