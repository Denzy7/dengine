#include "dengine-utils/thread.h"
#ifdef DENGINE_WIN32
#include <windows.h>
#endif
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
    pthread_cancel(thread->thr);
#endif
}
