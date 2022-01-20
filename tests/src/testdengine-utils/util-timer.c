#include <dengine-utils/timer.h>
#include <dengine-utils/logging.h>

#include <math.h> //fmod

double elapsed = 0;
int main()
{
    double count = 6000.0;
    dengineutils_logging_log("INFO::timer start %d secs", (int)(count / 1000.0));
    //quick init
    dengineutils_timer_update();

    while(elapsed < count)
    {
        dengineutils_timer_update();
        elapsed+=dengineutils_timer_get_delta();

        if(fmod(elapsed, 1000.0) == 0.0)
        {
            dengineutils_logging_log("%d", (int)((count - elapsed + 1000.0) / 1000.0));
        }
    }

    dengineutils_logging_log("INFO::timer done");
}
