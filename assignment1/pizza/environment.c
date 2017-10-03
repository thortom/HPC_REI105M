#include <stdlib.h>
#include <math.h>
#include "environment.h"

bool random_bool() {
    return rand() % 2;
}

bool is_hot() {
    return random_bool();
}

bool is_cold() {
    return random_bool();
}

bool phone_is_rinnging() {
    return random_bool();
}
