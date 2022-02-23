#ifndef TIMER_H
#define TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

void dengineutils_timer_set_current(double time);

void dengineutils_timer_update();

double dengineutils_timer_get_delta();

double dengineutils_timer_get_current();

#ifdef __cplusplus
}
#endif

#endif // TIMER_H
