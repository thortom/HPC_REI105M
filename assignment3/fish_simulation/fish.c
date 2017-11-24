#include "fish.h"
#include "logger.h"

void fish_group_constructor(fish_group *me, int group_number)
{
    me->group_number = group_number;
    /* random int between 10 and 20 */
    me->numb_fish = (rand() % 11) + 10;
}

void print_fish_group(fish_group *me)
{
    log_info("Fish group number: %d, contains: %d fish", me->group_number, me->numb_fish);
}

int get_current_direction(fish_group *me)
{
    /* Up, Down, Left, Right */
    int direction = rand() % 4;
    log_debug("Fish group: %d heading: %d", me->group_number, direction);
    return direction;
}
