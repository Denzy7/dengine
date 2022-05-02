#ifndef DENGINE_STATE_H
#define DENGINE_STATE_H

#include <dengine_config.h>
#include "dengine/macros.h"

/*
 * Locking mechanics for MT critical sections
 */

#if defined(DENGINE_MT)
    #if defined(DENGINE_HAVE_PTHREAD_H)
    #include <pthread.h>
    static pthread_mutex_t state_lock = PTHREAD_MUTEX_INITIALIZER;
    #else
    #error "Cannot implement mutex without pthread.h"
    #endif
#endif

DENGINE_INLINE void dengine_state_lock()
{
#if defined(DENGINE_MT) && defined(DENGINE_HAVE_PTHREAD_H)
    pthread_mutex_lock(&state_lock);
#endif
}

DENGINE_INLINE void dengine_state_unlock()
{
#if defined(DENGINE_MT) && defined(DENGINE_HAVE_PTHREAD_H)
    pthread_mutex_unlock(&state_lock);
#endif
}

#endif
