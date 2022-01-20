#ifndef RNG_H
#define RNG_H
#ifdef __cplusplus
extern "C" {
#endif
int dengineutils_rng_int(int max);

void dengineutils_rng_set_seed(unsigned int seed);

void dengineutils_rng_set_seedwithtime();
#ifdef __cplusplus
}
#endif
#endif // RNG_H
