#include <dengine-utils/thread.h>
#include <dengine-utils/logging.h>
#include <dengine-utils/timer.h>
#include <stdio.h> //scanf
#include <math.h> //fmod
typedef struct
{
    int a;
    float b;
    char c;
}somedata;

int run = 1;

void* startthr(void* arg)
{
    somedata* data = arg;
    dengineutils_logging_log("INFO::thread start %d %f %c",
                             data->a,
                             data->b,
                             data->c);

    dengineutils_timer_update();

    double delta = 0;
    static double elapsed = 0;
    while(elapsed < 10000.0)
    {
        dengineutils_timer_update();
        delta = dengineutils_timer_get_delta();
        elapsed += delta;
        if(fmod(elapsed, 1000.0) == 0)
            dengineutils_logging_log("INFO::I WILL NOW INTERRUPT!");
    }
    run = 0;
    return NULL;
}

int main(int argc, char *argv[])
{
    dengineutils_logging_log("INFO::Enter some text. A thread will interrupt you every second!");
    somedata data = {33, 33.3, '3'};
    Thread thr;
    dengineutils_thread_create(startthr, &data, &thr);
    char buf[256];
    while(run)
    {
        scanf("%256s", buf);
        dengineutils_logging_log("TODO::You entered : %s", buf);
    }

    return 0;
}
