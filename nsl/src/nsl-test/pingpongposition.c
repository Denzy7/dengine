#include <dengine/dengine.h>
#include <math.h>

int pingpongposition_update(Entity* entity)
{
    double current = dengineutils_timer_get_current();
    double pingpong = sin(current / 1000.0);
    entity->transform.position[0] = pingpong * 5.0;
    return 0;
}
