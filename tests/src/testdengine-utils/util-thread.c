#include <dengine-utils/thread.h>
#include <dengine-utils/logging.h>
#include <dengine-utils/timer.h>
#include <stdint.h> 
#include <stdio.h> //scanf
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

    /* we just need tame seconds precision, we'll have to 
     * account for lost of precison by getting the last 
     * time we interrupted so we dont interupt multiple times in a second*/
    static double elapsed = 0;
    uint32_t lastint = 0;
    while(elapsed < 10000.0)
    {
        dengineutils_timer_update();
        elapsed += dengineutils_timer_get_delta();
        if((uint32_t)elapsed % 1000 == 0 && lastint != (uint32_t)elapsed / 1000){
            dengineutils_logging_log("INFO::I WILL NOW INTERRUPT! %u", (uint32_t)elapsed / 1000);
            lastint = (uint32_t)elapsed / 1000;
        }
    }
    dengineutils_logging_log("WARNING::press enter to get your input!");
    run = 0;
    return NULL;
}

int main(int argc, char *argv[])
{
    Thread thr;
    somedata data = {33, 33.3, '3'};
    char buf[256];
    dengineutils_thread_create(startthr, &data, &thr);
    dengineutils_logging_log("INFO::Enter some text. A thread will interrupt you every second!");
    dengineutils_thread_wait(&thr);
    scanf("%256s", buf);
    dengineutils_logging_log("TODO::you entered: %s\n", buf);

    return 0;
}
