#include "physics.h"

const char* get_direction_string(int direction)
{
    static char* directions[] = {"UP", "DOWN", "LEFT", "RIGHT"}; 
    static char badFood[] = "Unknown";
    if (direction < 0 || direction > 3) 
        return badFood;
    else
        return directions[direction];
}
