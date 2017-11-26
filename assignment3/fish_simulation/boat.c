#include "boat.h"

void boat_constructor(Boat *me, int number)
{
    me->number = number;
    me->numb_fish_caught = 0;
    me->direction = -1;
}

void print_boat(Boat *me)
{
    log_info("Boat number: %d, has caught: %d fish", me->number, me->numb_fish_caught);
}

void update_boat_direction(Boat *me, int coords[])
{
    if (me->numb_fish_caught)
    {
        if (coords[0] == 0 && coords[1] == 0)
        {
            /* Unload the caught fish */
            me->direction = rand() % 4;
            log_debug("Boat: %d unloaded %d fish and now heading: %s",
                    me->number, me->numb_fish_caught, get_direction_string(me->direction));
            me->numb_fish_caught = 0;
            return;
        }
        else if (coords[0] != 0)
        {
            me->direction = UP;
        }
        else if (coords[1] != 0)
        {
            me->direction = LEFT;
        }
        log_debug("Boat: %d with %d fish, heading to harbor in the direction: %s",
                        me->number, me->numb_fish_caught, get_direction_string(me->direction));
    }
    else
    {
        /* Up, Down, Left, Right */
        me->direction = rand() % 4;
        log_debug("Boat: %d heading: %s", me->number, get_direction_string(me->direction));
        return;
    }
}

int boat_data_equal(Boat data_1, Boat data_2)
{
    if (data_1.number == data_2.number &&
        data_1.numb_fish_caught == data_2.numb_fish_caught)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
