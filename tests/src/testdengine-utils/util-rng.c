#include <dengine-utils/rng.h>
#include <dengine-utils/logging.h>

void tenrandomnumbersnow()
{
    int r;
    for(int i = 0; i < 10; i++)
    {
        r = dengineutils_rng_int(10);
        dengineutils_logging_log("%d", r);
    }
    dengineutils_logging_log("INFO::generated 10 random numbers");
}

int main()
{
    tenrandomnumbersnow();
    //Oh no!, Why are they all the same?

    //let's try a seed
    dengineutils_rng_set_seed(123);
    tenrandomnumbersnow();

    //still the same?
    //seed with time!
    dengineutils_rng_set_seedwithtime();
    tenrandomnumbersnow();

    return 0;
}
