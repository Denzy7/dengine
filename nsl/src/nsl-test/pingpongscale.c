#include <dengine/dengine.h>
#include <math.h>
#include <stdlib.h>

int pingpongscale_update(Entity* entity)
{
    double current = dengineutils_timer_get_current();
    double pingpong = sin(current / 1000.0);
    entity->transform.scale[2] = fabs(pingpong) * 1.3;
    entity->transform.scale[1] = fabs(pingpong) * 0.3;
    return 0;
}
