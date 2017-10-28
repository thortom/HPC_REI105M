#ifndef ELEVATOR_H_INCLUDED
#define ELEVATOR_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <unistd.h>
#include "logger.h"

#define N_DATA_ITEMS 3
#define N_PERSONS_ALLOWED 7

typedef struct ElevatorIdentity {
    int rank;
    int current_level;
    int is_running;
    int persons_allowed[N_PERSONS_ALLOWED];
    int numb_persons;
} Elevator;

typedef struct ElevatorDataIdentity 
{
    int person_rank;
    int from;
    int to;
} ElevatorData;

/* Global variables */
ElevatorData ELEVATOR_NULL_DATA;
ElevatorData ELEVATOR_SHUTDOWN_SIGNAL;
MPI_Datatype mpi_elevator_data_type;

void init_elevator_data();
void elevator_data_free();
int elevator_data_equal(ElevatorData data_1, ElevatorData data_2);
void Elevator_constructor(Elevator *me, int rank, int numb_persons);
void print_elevator_data(ElevatorData data);
void print_elevator(Elevator *me);
void start(Elevator *me);

#endif