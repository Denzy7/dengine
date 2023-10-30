#ifndef TIMER_H
#define TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

/* none of these are thread safe, use get_current_t
 * for safety
 */

void dengineutils_timer_set_current(double time);

void dengineutils_timer_update();

double dengineutils_timer_get_delta();

double dengineutils_timer_get_current();

/* hi res timer, not necesarrily the time since
 * boot or time since epoch, its platform 
 * dependent
 */
int dengineutils_timer_get_current_r(double* res);

#ifdef __cplusplus
}
#endif

#endif // TIMER_H
