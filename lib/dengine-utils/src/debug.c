#include "dengine-utils/debug.h"

#include "dengine-utils/logging.h"
#include "dengine-utils/timer.h"

#include "dengine_config.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <signal.h>

void _dengineutils_debug_hand_segv(int sigsegv);
void _dengineutils_debug_hand_abrt(int sigabrt);

char** trace = NULL;
char* fmt=NULL;
uint32_t traceptr=0;

void dengineutils_debug_init()
{
    trace=malloc(sizeof(char*)*DENGINE_DEBUG_TRACESIZE);
    for (int i = 0; i < DENGINE_DEBUG_TRACESIZE; i++) {
        trace[i]=malloc(DENGINE_DEBUG_TRACESTRLN);
        snprintf(trace[i],DENGINE_DEBUG_TRACESTRLN,"...");
    }
    fmt=malloc(DENGINE_DEBUG_TRACESTRLN);

    signal(SIGSEGV,_dengineutils_debug_hand_segv);
    signal(SIGABRT,_dengineutils_debug_hand_abrt);
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

    if(traceptr==DENGINE_DEBUG_TRACESIZE)
        traceptr=0;

    snprintf(trace[traceptr],DENGINE_DEBUG_TRACESTRLN,"%s",str);
    traceptr++;
}

void dengineutils_debug_trace_dump()
{
    if(!trace)
        return;

    FILE* f=fopen(DENGINE_DEBUG_TRACEFILE,"a");
    dengineutils_timer_update();

    for (uint32_t i = DENGINE_DEBUG_TRACESIZE; i > 0; i--) {
        snprintf(fmt,DENGINE_DEBUG_TRACESTRLN,"%3u: %s %s",i-1,trace[i-1],i==traceptr?"<==":"");
        dengineutils_logging_log("%s",fmt);
        if(f)
        {
            fprintf(f,"%s\n",fmt);
        }
    }
}

void dengineutils_debug_enter(const char* function,const char* file,const int line)
{
    if(!trace)
        return;

    snprintf(fmt,DENGINE_DEBUG_TRACESTRLN,"%s, %s:%d",function,strstr(file,"dengine"),line);
    dengineutils_debug_trace_push(fmt);
}

void _dengineutils_debug_hand_segv(int sigsegv)
{
    dengineutils_debug_trace_dump();
    dengineutils_logging_log("ERROR::SIGSEGV.trace dumped. exiting...");
    exit(sigsegv);
}

void _dengineutils_debug_hand_abrt(int sigabrt)
{
    dengineutils_debug_trace_dump();
    dengineutils_logging_log("ERROR::SIGABRT.trace dumped. exiting...");
    exit(sigabrt);
}
