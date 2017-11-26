#ifndef BOAT_H_INCLUDED
#define BOAT_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include "logger.h"
#include "physics.h"

typedef struct boat_identity {
    int number;
    int numb_fish_caught;
    int direction;
} Boat;

void boat_constructor(Boat *me, int number);
void print_boat(Boat *me);
void update_boat_direction(Boat *me, int coords[]);
int boat_data_equal(Boat data_1, Boat data_2);

#endif
