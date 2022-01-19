#include "rng.h"

#include <stdlib.h> //rand_r()
#include <time.h> //time_t, time()
unsigned int _seed = 0;

void dengineutils_rng_set_seed(unsigned int seed)
{
    _seed = seed;
}

void dengineutils_rng_set_seedwithtime()
{
    time_t t;
    time(&t);

    _seed = t;
}

int dengineutils_rng_int(int max)
{
    return rand_r(&_seed) % max;
}
