#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include "person.h"

int ACTION_TIME = 1000 * 1000;  /* One second*/

void Person_constructor(Person *me, int rank, int gender,
                            int group_rank) {
    me->rank = rank;
    me->gender = gender;
    me->group_rank = group_rank;
    /* random int between 2 and 20 */
    me->pepperoni = (rand() % 19) + 2;
    /* random int between 6 and 10 */
    me->pizza_size = (rand() % 5) + 6;
    me->is_window_open = false;
}

int get_pepperoni_count(Person *me) {
    return me->pepperoni;
}

bool done_with_pizza(Person *me) {
    if (me->pizza_size == 0)
        return true;
    return false;
}

void take_bite_of_pizza(Person *me) {
    if (me->pizza_size > 0) {
        usleep(ACTION_TIME);
        me->pizza_size -= 1;
    }
}

void open_window(Person *me) {
    if (me->is_window_open) {
        printf("The window is already open\n");
    }
    else {
        usleep(ACTION_TIME);
        me->is_window_open = true;
    }
}

void close_window(Person *me) {
    if (!me->is_window_open) {
        printf("The window is already closed\n");
    }
    else {
        usleep(ACTION_TIME);
        me->is_window_open = true;
    }
}

void answer_phone(Person *me) {
    printf("Answering the phone\n");
    usleep(ACTION_TIME);
    printf("Done talking\n");
}

char* get_group_name(Gender gender) {
    if (gender == female)
        return "female";
    else
        return "male";
}

void get_my_name(Person *me, char* my_name) {
    if (me->rank == 0)
        snprintf(my_name, 20, "the host");
    else    
        snprintf(my_name, 20, "%s, %d",
                        get_group_name(me->gender), me->group_rank);
}

void print_info(Person *me) {
    if (me->rank == 0)
        printf("The host with world rank 0\n");
    else
        printf("Person with world rank %d and group rank %d in %s group\n",
                    me->rank, me->group_rank, get_group_name(me->gender));
    printf("    I got %d pepperoni on my pizza\n", me->pepperoni);
    printf("    My pizza is %d bites\n", me->pizza_size);
}
