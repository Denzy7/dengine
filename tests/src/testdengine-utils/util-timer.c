#include <stdint.h>
#include <dengine-utils/timer.h>
#include <dengine-utils/logging.h>
int main()
{
    /* only need second precision */
    double count = 6000.0, elapsed = 0.0;
    dengineutils_logging_log("INFO::timer start %d secs", (uint32_t)(count / 1000));
    //quick init
    dengineutils_timer_update();

    while(elapsed < count)
    {
        dengineutils_timer_update();
        elapsed += dengineutils_timer_get_delta();

        if((uint32_t)elapsed % 1000 == 0)
        {
            dengineutils_logging_log("%u", (uint32_t)((count - elapsed + 1000.0) / 1000.0));
        }
    }

    dengineutils_logging_log("INFO::timer done");
}
