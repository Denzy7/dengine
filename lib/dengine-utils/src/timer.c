#include "dengine-utils/timer.h"
#include "dengine_config.h"
#ifdef DENGINE_LINUX
#include <time.h> //clock_gettime
#endif
#ifdef DENGINE_WIN32
#include <windows.h>
#endif

double _last, _delta, _current, _first = 0.0;

void dengineutils_timer_set_current(double time)
{
    _current = time;
    _first = _current;
    _delta = _current - _last;
    _last = _current;
}

void dengineutils_timer_update()
{
    double t = 0.0;
    dengineutils_timer_get_current_r(&t);
    _current = t;
    _delta = _current - _last;
    _last = _current;

    if(_first == 0.0)
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

int dengineutils_timer_get_current_r(double* res)
{
    int ret = 0;
#ifdef DENGINE_WIN32
    LARGE_INTEGER pf, pc;
    QueryPerformanceFrequency(&pf);
    /* pf could be cached but where to keep this rentrant? 
     * TODO: thread specific storage? */
    QueryPerformanceCounter(&pc);
    *res = pc.QuadPart * 1e3 / pf.QuadPart;
    ret = 1;
#endif
#ifdef DENGINE_LINUX
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    *res = 1000.0 * spec.tv_sec + (double)spec.tv_nsec / 1e6;
#endif
    return ret;
}
