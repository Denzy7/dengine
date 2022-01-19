#include "timer.h"

double _last, _delta, _current;
#include <time.h> //clock_gettime

struct timespec spec;

void dengineutils_timer_set_current(double time)
{
    _current = time;
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
}

double dengineutils_timer_get_delta()
{
    return _delta;
}
