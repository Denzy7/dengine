#include <dengine/dengine.h>

int moveduck_start(Entity* entity)
{
    printf("Start invoked from C to NSL\n");
    printf("files: %s\n", dengineutils_filesys_get_filesdir());
    return 0;
}

int moveduck_update(Entity* entity)
{
    static const double speed = 4.0f;
    double delta_s = dengineutils_timer_get_delta() / 1000.0;
    if(dengine_input_get_key('A'))
       entity->transform.position[0] -= delta_s * speed;
    if(dengine_input_get_key('D'))
       entity->transform.position[0] += delta_s * speed;
    if(dengine_input_get_key('E'))
       entity->transform.position[1] += delta_s * speed;
    if(dengine_input_get_key('C'))
       entity->transform.position[1] -= delta_s * speed;
    if(dengine_input_get_key('W'))
       entity->transform.position[2] -= delta_s * speed;
    if(dengine_input_get_key('S'))
       entity->transform.position[2] += delta_s * speed;
    if(dengine_input_get_key('Z'))
       entity->transform.rotation[1] += delta_s * speed * 30.0;
    if(dengine_input_get_key('X'))
       entity->transform.rotation[1] -= delta_s * speed * 30.0;

    return 0;
}
