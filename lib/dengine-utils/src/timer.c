#include "dengine-utils/timer.h"

double _last, _delta, _current, _first = 0.0;
#include <time.h> //clock_gettime

struct timespec spec;

#include "dengine_config.h"
//Quick and dirty clock_gettime for Win32
//https://github.com/Alexpux/mingw-w64/blob/master/mingw-w64-libraries/winpthreads/src/clock.c
#ifdef DENGINE_WIN32
#include <windows.h>
#include <errno.h>
#include <stdint.h>

#define POW10_7                 10000000
#define POW10_9                 1000000000

/* Number of 100ns-seconds between the beginning of the Windows epoch
 * (Jan. 1, 1601) and the Unix epoch (Jan. 1, 1970)
 */
#define DELTA_EPOCH_IN_100NS    INT64_C(116444736000000000)

int clock_gettime(clockid_t clock_id, struct timespec *tp)
{
    unsigned __int64 t;
    LARGE_INTEGER pf, pc;
    union {
        unsigned __int64 u64;
        FILETIME ft;
    }  ct, et, kt, ut;

    switch(clock_id) {
    case CLOCK_REALTIME:
        {
            GetSystemTimeAsFileTime(&ct.ft);
            t = ct.u64 - DELTA_EPOCH_IN_100NS;
            tp->tv_sec = t / POW10_7;
            tp->tv_nsec = ((int) (t % POW10_7)) * 100;

            return 0;
        }

    case CLOCK_MONOTONIC:
        {
            if (QueryPerformanceFrequency(&pf) == 0)
                return 1;

            if (QueryPerformanceCounter(&pc) == 0)
                return 1;

            tp->tv_sec = pc.QuadPart / pf.QuadPart;
            tp->tv_nsec = (int) (((pc.QuadPart % pf.QuadPart) * POW10_9 + (pf.QuadPart >> 1)) / pf.QuadPart);
            if (tp->tv_nsec >= POW10_9) {
                tp->tv_sec ++;
                tp->tv_nsec -= POW10_9;
            }

            return 0;
        }

    case CLOCK_PROCESS_CPUTIME_ID:
        {
        if(0 == GetProcessTimes(GetCurrentProcess(), &ct.ft, &et.ft, &kt.ft, &ut.ft))
            return 1;
        t = kt.u64 + ut.u64;
        tp->tv_sec = t / POW10_7;
        tp->tv_nsec = ((int) (t % POW10_7)) * 100;

        return 0;
        }

    case CLOCK_THREAD_CPUTIME_ID:
        {
            if(0 == GetThreadTimes(GetCurrentThread(), &ct.ft, &et.ft, &kt.ft, &ut.ft))
                return 1;
            t = kt.u64 + ut.u64;
            tp->tv_sec = t / POW10_7;
            tp->tv_nsec = ((int) (t % POW10_7)) * 100;

            return 0;
        }

    default:
        break;
    }

    return 1;
}
#endif


void dengineutils_timer_set_current(double time)
{
    _current = time;
    _first = _current;
    _delta = _current - _last;
    _last = _current;
}

void dengineutils_timer_update()
{
    clock_gettime(CLOCK_REALTIME, &spec);

    double time = 1000.0 * spec.tv_sec + (double)spec.tv_nsec / 1e6;
    _current = time;
    _delta = _current - _last;
    _last = _current;

    if(_first == 0)
        _first = _current;
}

double dengineutils_timer_get_delta()
{
    return _delta;
}

double dengineutils_timer_get_current()
{
    return _current - _first;
}
