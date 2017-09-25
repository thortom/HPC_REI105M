#include <stdio.h>
#include <stdlib.h>
/*Header file for bool functions */
typedef int bool;
#define true 1
#define false 0

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

