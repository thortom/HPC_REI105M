#ifndef PERSON_H_INCLUDED
#define PERSON_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <unistd.h>
#include "logger.h"
#include "elevator.h"

typedef struct PersonIdentity {
    int rank;
    char* name;
    int work_level;
} Person;

void Person_constructor(Person *me, int rank, char* name);
void print_person(Person *me);
int going_to_work(Person *me);
int going_home(Person *me);
void browsing_phone(Person *me);
void request_elevator(Person *me, int from, int to, int elevator_rank);
void shutdown_the_elevator(Person *me, int elevator_rank);

#endif
