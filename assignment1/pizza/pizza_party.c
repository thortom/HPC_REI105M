#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include "person.h"
#include "environment.h"

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


