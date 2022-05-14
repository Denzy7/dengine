#include <dengine/dengine.h>

int constantrotation_update(Entity* entity)
{
    static const double speed = 4.0;
    double delta = dengineutils_timer_get_delta();
    double delta_s = delta / 1000.0;
    entity->transform.rotation[1] += delta_s * speed;

    return 0;
}
