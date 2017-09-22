#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

typedef int Gender;
#define male 1
#define female 2

typedef int bool;
#define true 1
#define false 0

/* global variable declaration */
int action_time = 1000 * 1000;  /* One second*/

/* TODO: Move Person in to separate file */
typedef struct PersonIdentity {
    int rank;
    Gender gender;
    int group_rank;
    int pepperoni;
    int pizza_size;
    int is_window_open;
} Person;

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
        usleep(action_time);
        me->pizza_size -= 1;
    }
}

void open_window(Person *me) {
    if (me->is_window_open) {
        printf("The window is already open\n");
    }
    else {
        usleep(action_time);
        me->is_window_open = true;
    }
}

void close_window(Person *me) {
    if (!me->is_window_open) {
        printf("The window is already closed\n");
    }
    else {
        usleep(action_time);
        me->is_window_open = true;
    }
}

void answer_phone(Person *me) {
    printf("Answering the phone\n");
    usleep(action_time);
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

/* TODO: Maybe move these global functions in to separate file as well*/
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


int main (int argc, char *argv[])
{
    int world_rank, world_size;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int group;
    /* Host is in group 0, men in 1 and women in 2*/
    if (world_rank == 0) {
        group = 0;
    }
    else {
        /* This maps world_rank 1 as the first in group 1 */
        /* and world_rank 2 as the fist in group 2 */
        group = (world_rank % 2) + 1;
    }

    MPI_Comm group_comm;
    MPI_Comm_split(MPI_COMM_WORLD, group, world_rank, &group_comm);

    int group_rank, group_size;
    MPI_Comm_size(group_comm, &group_size);
    MPI_Comm_rank(group_comm, &group_rank);

    /* Putting time as seed for the random generator */
    srand(time(NULL) + world_rank);

    Person me;
    Person_constructor(&me, world_rank, group, group_rank);

    print_info(&me);

    int tag = 1;
    int total_pepperoni_count;
    int local_pepperoni_count = get_pepperoni_count(&me);
    if (world_rank != 0)
    {
        /* group_rank 0 sums up for it's room*/
        MPI_Reduce(&local_pepperoni_count, &total_pepperoni_count,
                    1, MPI_INT, MPI_SUM, 0, group_comm);
    }
    if (group_rank == 0 && group != 0)
    {
        printf("Sum of pepperoni for group %d\n", total_pepperoni_count);
        MPI_Send(&total_pepperoni_count, 1, MPI_INT, 0, tag,
                            MPI_COMM_WORLD);
    }
    else if (world_rank == 0)
    {
        MPI_Status Stat;
        int pepperoni_group_1, pepperoni_group_2;
        MPI_Recv(&pepperoni_group_1, 1, MPI_INT, 1, tag,
                            MPI_COMM_WORLD, &Stat);
        MPI_Recv(&pepperoni_group_2, 1, MPI_INT, 2, tag,
                            MPI_COMM_WORLD, &Stat);
        total_pepperoni_count = local_pepperoni_count + pepperoni_group_1 + pepperoni_group_2;
        printf("Sum of pepperoni global %d\n", total_pepperoni_count);
    }

    
    while (!done_with_pizza(&me))
    {
        if (world_rank == 0)
        {
            if (is_hot(&me))
            {
                open_window(&me);
            }
            if (is_cold(&me))
            {
                close_window(&me);
            }
            if (phone_is_rinnging(&me))
            {
                answer_phone(&me);
            }
        }
        take_bite_of_pizza(&me);
        char name[20];
        get_my_name(&me, name);
        printf("%s took one bite\n", name);
    }

    if (world_rank != 0)
    {
        printf("Node rank %d is waiting for the host\n", world_rank);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    usleep(10000);          /* 10 millisecond sleep to fix output */
    printf("All done, party time, whoop whoop!\n");

    MPI_Comm_free(&group_comm);
    MPI_Finalize();
    return 0;
}

