#ifndef DEBUG_H
#define DEBUG_H

#define DENGINE_DEBUG_ENTER dengineutils_debug_enter(__FUNCTION__,__FILE__,__LINE__)

void dengineutils_debug_init();

void dengineutils_debug_terminate();

void dengineutils_debug_enter(const char* function,const char* file,const int line);

void dengineutils_debug_trace_push(const char* str);

void dengineutils_debug_trace_dump();

#endif // DEBUG_H
