#include <dengine-utils/timer.h>
#include <dengine-utils/logging.h>

double elapsed = 0;

int main()
{
    dengineutils_logging_log("INFO::timer start 5 secs");
    //quick init
    dengineutils_timer_update();
    while(elapsed < 5000.0)
    {
        dengineutils_timer_update();
        elapsed+=dengineutils_timer_get_delta();
    }

    dengineutils_logging_log("INFO::timer done");
}
