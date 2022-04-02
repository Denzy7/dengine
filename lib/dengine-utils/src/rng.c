#include "dengine-utils/rng.h"

#include <stdlib.h> //rand()
#include <time.h> //time_t, time()

void dengineutils_rng_set_seed(unsigned int seed)
{
    srand(seed);
}

void dengineutils_rng_set_seedwithtime()
{
    time_t t;
    time(&t);

    srand(t);
}

int dengineutils_rng_int(int max)
{
    return rand() % max;
}
