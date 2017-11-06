#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <unistd.h>
#include "logger.h"
#include "elevator.h"
#include "person.h"

char * person_names[] = {
    "Noah",
    "Emma",
    "Stella",
    "James",
    "Ezra",
    "Ace",
    "Isabella",
};

int main (int argc, char *argv[])
{
    init_logger();

    /* This is a three level building with only the lobby on level 0
        and offices on levels 1, 2, 3 */
    int rank, world_size;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /* Putting time as seed for the random generator */
    srand(time(NULL) + rank);

    int elevator_rank = 0;
    int group;
    /* Host is in group 0, men in 1 and women in 2*/
    if (rank == elevator_rank)
    {
        /* Elevator group */
        group = 0;
    }
    else
    {
        /* Workers group */
        group = 1;
    }

    MPI_Comm group_comm;
    MPI_Comm_split(MPI_COMM_WORLD, group, rank, &group_comm);

    int group_rank;
    MPI_Comm_rank(group_comm, &group_rank);

    init_elevator_data();
    if (rank == elevator_rank)
    {
        Elevator me;
        Elevator_constructor(&me, elevator_rank, world_size - 1);

        print_elevator(&me);
        start(&me);
    }
    else
    {
        Person me;
        Person_constructor(&me, rank, person_names[rank - 1]);

        print_person(&me);
        
        int level;
        if (rank != elevator_rank)
        {
            level = going_to_work(&me);
            request_elevator(&me, 0, level, elevator_rank);

            log_info("%s is working... working...", me.name);
            sleep(4);
        }

        if (rank != elevator_rank)
        {
            level = going_home(&me);
            request_elevator(&me, level, 0, elevator_rank);
        }
        
        log_info("%s is done working", me.name);
        MPI_Barrier(group_comm);

        sleep(1);
        if (group_rank == 0)
        {
            shutdown_the_elevator(&me, elevator_rank);
            log_info("The office building is closed");
        }
    }

    MPI_Comm_free(&group_comm);
    MPI_Finalize();

    return 0;
}
