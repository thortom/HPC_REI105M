#include "person.h"

void Person_constructor(Person *me, int rank, char* name)
{
    me->rank = rank;
    me->name = name;
    /* random int between 1 and 3 */
    me->work_level = (rand() % 3) + 1;
}

void print_person(Person *me)
{
    log_info("Person: %s, Rank: %d, Work Level: %d", me->name, me->rank, me->work_level);
}

int going_to_work(Person *me)
{
    log_info("%s going to work", me->name);
    return me->work_level;
}

int going_home(Person *me)
{
    log_info("%s going home", me->name);
    return me->work_level;
}

void browsing_phone(Person *me)
{
    log_info("%s I'm browsing my phone", me->name);
    sleep(1);
}

void request_elevator(Person *me, int from, int to, int elevator_rank)
{
    log_info("%s is requesting the elevator...", me->name);

    MPI_Status status;
    MPI_Request request;
    int tag = 1;

    ElevatorData data = {me->rank, from, to};

    /* Send the transfer data to the elevator */
    MPI_Isend(&data, 1, mpi_elevator_data_type, elevator_rank, tag, MPI_COMM_WORLD, &request);

    int flag = 0;
    while (!flag)
    {
        /* Check regularly whether the elevator is coming */
        MPI_Test(&request, &flag, &status);
        browsing_phone(me);
    }
    log_info("%s I'm going from: %d, to to: %d", me->name, from, to);
    log_info("%s the elevator is coming soon for me", me->name);

    int travel_time;
    MPI_Irecv(&travel_time, 1, MPI_INT, elevator_rank, tag, MPI_COMM_WORLD, &request);

    flag = 0;
    while (!flag)
    {
        MPI_Test(&request, &flag, &status);
        browsing_phone(me);
    }

    log_info("%s the elevator has arrived for me", me->name);
    sleep(travel_time);
    log_info("%s arrived at my level", me->name);
}

void shutdown_the_elevator(Person *me, int elevator_rank)
{
    int tag = 1;
    log_info("%s is shutting down the elevator", me->name);
    MPI_Send(&ELEVATOR_SHUTDOWN_SIGNAL, 1, mpi_elevator_data_type, elevator_rank, tag, MPI_COMM_WORLD);
}
