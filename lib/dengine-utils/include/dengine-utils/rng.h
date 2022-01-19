#ifndef RNG_H
#define RNG_H

int dengineutils_rng_int(int max);

void dengineutils_rng_set_seed(unsigned int seed);

void dengineutils_rng_set_seedwithtime();
#endif // RNG_H
