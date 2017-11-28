#include "fish.h"

void fish_group_constructor(Fish_group *me, int group_number)
{
    me->group_number = group_number;
    /* random int between 10 and 20 */
    me->numb_fish = (rand() % 11) + 10;
    me->direction = -1;
}

void print_fish_group(Fish_group *me)
{
    log_info("Fish group number: %d, contains: %d fish", me->group_number, me->numb_fish);
}

void update_fish_group(Fish_group *me)
{
    /* Inrease the fish population by one or none */
    me->numb_fish += rand() % 2;

    /* Up, Down, Left, Right */
    me->direction = rand() % 4;
    log_debug("Fish group: %d heading: %s",
        me->group_number, get_direction_string(me->direction));
}

int fish_data_equal(Fish_group data_1, Fish_group data_2)
{
    if (data_1.group_number == data_2.group_number &&
        data_1.numb_fish == data_2.numb_fish)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
