#include "dengine-utils/debug.h"

#include "dengine-utils/logging.h"
#include "dengine-utils/timer.h"
#include "dengine-utils/filesys.h"
#include "dengine-utils/thread.h" /* mutex */
#include "dengine-utils/macros.h" /* arysz */

#include "dengine_config.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h> //ctime

#include <signal.h>

void _dengineutils_debug_hand_termandexit(int sig);

char** trace = NULL;
char* fmt=NULL;
uint32_t traceptr=0;
Mutex tracemutex;

struct _sigstr
{
    int sig;
    const char* str;
};

static const struct _sigstr _sigtable_exit[] =
{
    {SIGSEGV, "Segmentation fault"},
    {SIGABRT, "Abnormal execution"},
};

void dengineutils_debug_init()
{
    trace=malloc(sizeof(char*)*DENGINE_DEBUG_TRACESIZE);
    for (int i = 0; i < DENGINE_DEBUG_TRACESIZE; i++) {
        trace[i]=malloc(DENGINE_DEBUG_TRACESTRLN);
        snprintf(trace[i],DENGINE_DEBUG_TRACESTRLN,"...");
    }
    fmt=malloc(DENGINE_DEBUG_TRACESTRLN);

    for(int i = 0; i < DENGINE_ARY_SZ(_sigtable_exit); i++)
    {
        signal(_sigtable_exit[i].sig,_dengineutils_debug_hand_termandexit);
    }

    dengineutils_thread_mutex_create(&tracemutex);
}

void dengineutils_debug_terminate()
{
    if(!trace)
        return;

    for (int i = 0; i < DENGINE_DEBUG_TRACESIZE; i++) {
        if(trace[i])
            free(trace[i]);
    }
    free(trace);
    free(fmt);
    traceptr=0;
}

void dengineutils_debug_trace_push(const char* str)
{
    if(!trace)
        return;

    dengineutils_thread_mutex_lock(&tracemutex);

    if(traceptr==DENGINE_DEBUG_TRACESIZE)
        traceptr=0;

    snprintf(trace[traceptr],DENGINE_DEBUG_TRACESTRLN,"%s",str);
    traceptr++;

    dengineutils_thread_mutex_unlock(&tracemutex);
}

void dengineutils_debug_trace_dump()
{
    if(!trace)
        return;

    const char* tracefile = DENGINE_DEBUG_TRACEFILE;
    char buf[2048];
    if(dengineutils_filesys_isinit())
    {
        snprintf(buf, sizeof(buf), "%s/logs/%s", dengineutils_filesys_get_filesdir_dengine(), DENGINE_DEBUG_TRACEFILE);
        if(fopen(buf, "r"))
            tracefile = buf;
    }

    FILE* f=fopen(tracefile,"a");
    dengineutils_timer_update();

    for (uint32_t i = DENGINE_DEBUG_TRACESIZE; i > 0; i--) {
        snprintf(fmt,DENGINE_DEBUG_TRACESTRLN,"%3u: %s %s",i-1,trace[i-1],i==traceptr?"<==":"");
        dengineutils_logging_log("%s",fmt);
        if(f)
        {
            fprintf(f,"%s\n",fmt);
        }
    }
    if(f)
    {
        time_t t = time(NULL);
        fprintf(f, "%s\n", ctime(&t));
        fclose(f);

        dengineutils_logging_log("WARNING::Trace dumped to %s", tracefile);
    }
}

void dengineutils_debug_enter(const char* function,const char* file,const int line)
{
    if(!trace)
        return;

    snprintf(fmt,DENGINE_DEBUG_TRACESTRLN,"%s, %s:%d",function,strstr(file,"dengine"),line);
    dengineutils_debug_trace_push(fmt);
}

void _dengineutils_debug_hand_termandexit(int sig)
{
    const char* sigstr = "SIG____ (Not implemented!)";
    for(int i = 0; i < DENGINE_ARY_SZ(_sigtable_exit); i++)
    {
        if(sig == _sigtable_exit[i].sig)
            sigstr = _sigtable_exit[i].str;
    }

    dengineutils_debug_trace_dump();
    dengineutils_logging_log("ERROR::%s, sig=%d\nTrace duming, exiting...", sigstr, sig);

    exit(sig + 128);
}
