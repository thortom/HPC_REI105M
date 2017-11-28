#ifndef FISH_GROUP_H_INCLUDED
#define FISH_GROUP_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include "logger.h"
#include "physics.h"

typedef struct fish_group_identity {
    int group_number;
    int numb_fish;
    int direction;
} Fish_group;

void fish_group_constructor(Fish_group *me, int group_number);
void print_fish_group(Fish_group *me);
void update_fish_group(Fish_group *me);
int fish_data_equal(Fish_group data_1, Fish_group data_2);

#endif
