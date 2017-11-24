#ifndef FISH_GROUP_H_INCLUDED
#define FISH_GROUP_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>

typedef struct fish_group_identity {
    int group_number;
    int numb_fish;
} fish_group;

void fish_group_constructor(fish_group *me, int group_number);
void print_fish_group(fish_group *me);
int get_current_direction(fish_group *me);

#endif
