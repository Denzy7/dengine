#include <dengine/dengine.h>

double lastmx, lastmy;
int scenecamera_start(Entity* e)
{
    lastmx = dengine_input_get_mousepos_x();
    lastmy = dengine_input_get_mousepos_y();
    return 1;
}

int scenecamera_update(Entity* entity)
{
    double delta_s = dengineutils_timer_get_delta() / 1000.0;
    double mx = dengine_input_get_mousepos_x();
    double my = dengine_input_get_mousepos_y();
    double mscr = dengine_input_get_mousescroll_y();
    double dx = lastmx - mx;
    double dy = lastmy - my;

    if(dengine_input_get_mousebtn(DENGINE_INPUT_MSEBTN_SECONDARY))
    {
        entity->transform.rotation[0] += (dy * delta_s * -5.0f);
        entity->transform.rotation[1] += (dx * delta_s * -5.0f);

        if(mscr )
        {
            vec3 front;
            denginescene_ecs_get_front(entity, front);
            /* we wanna realign the homogenous part so we can easily
             * go back and forth with scroll wheel*/
            glm_vec3_sub(front, entity->transform.position, front);
            glm_vec3_scale(front, mscr, front);
            glm_vec3_add(front, entity->transform.position, front);
            glm_vec3_lerp(entity->transform.position, front, delta_s * 30.0f, entity->transform.position);
        }
    }
    lastmx = mx;
    lastmy = my;
    return 1;
}
