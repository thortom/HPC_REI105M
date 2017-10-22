#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

/* Elevator.h */
#define N_DATA_ITEMS 3
#define N_PERSONS_ALLOWED 7

typedef struct ElevatorIdentity {
    int rank;
    int current_level;
    int is_running;
    int persons_allowed[N_PERSONS_ALLOWED];
    int numb_persons;
} Elevator;

struct ElevatorDataIdentity 
{
    int size;
    int null_data[N_DATA_ITEMS];
    int shutdown_signal[N_DATA_ITEMS];
    int person_rank;
    int from;
    int to;
} ElevatorData_default = {N_DATA_ITEMS, {999, 999, 999}, {-1, -1, -1}, 0, 1, 2};
typedef struct ElevatorDataIdentity ElevatorData;

void Elevator_constructor(Elevator *me, int rank, int numb_persons);
void print_elevator(Elevator *me);
void start(Elevator *me);

/* Elevator.h Ends */

/* Elevator.c */
void Elevator_constructor(Elevator *me, int rank, int numb_persons) {
    me->rank = rank;
    me->current_level = 0;
    me->is_running = 1;
    int i;
    for (i = 0; i < numb_persons; i++)
        me->persons_allowed[i] = i;
    me->numb_persons = numb_persons;
}

void print_elevator(Elevator *me)
{
    printf("Elevator:,\n\
        Rank: %d,\n\
        Current Level: %d\n", me->rank, me->current_level);
}

void start(Elevator *me)
{
    MPI_Status status;
    int i, tag = 1, flag = 0;
    int diff;
    ElevatorData transfer_data = ElevatorData_default;
    int data[transfer_data.size];
    while (me->is_running)
    {
        for (i = 1; i <= me->numb_persons; i++)
        {
            /* TODO: This... */
            
            printf("Probing for i=%d\n", i);
            MPI_Iprobe(i, tag, MPI_COMM_WORLD, &flag, &status);
            sleep(1);       /* Preventing to frequent probing */

            /*
            int MPI_Iprobe(int source, int tag, MPI_Comm comm, int *flag,
                   MPI_Status *status)
            int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source,
                int tag, MPI_Comm comm, MPI_Status *status)
             */

            if (flag == 1)
            {
                MPI_Recv(&data, 3, MPI_INT, i, tag, MPI_COMM_WORLD, &status);

                diff = memcmp(data, transfer_data.shutdown_signal, transfer_data.size);
                if (diff == 0)
                {
                    me->is_running = 0;
                    printf("The elevator is shutting down\n");
                    break;
                }
                else
                {
                    /* TODO: Accept the first person here*/
                    ;
                }
            }
        }
    }
}

/* Elevator.c Ends */

/* Person.h */
typedef struct PersonIdentity {
    int rank;
    char* name;
    int work_level;
} Person;

void print_person(Person *me);
int going_to_work(Person *me);
int going_home(Person *me);
void request_elevator(Person *me, int from, int to, int elevator_rank);
void shutdown_the_elevator(Person *me, int elevator_rank);
/* Person.h Ends */

/* Person.c */
void Person_constructor(Person *me, int rank, char* name) {
    me->rank = rank;
    me->name = name;
    /* random int between 1 and 3 */
    me->work_level = (rand() % 3) + 1;
}

void print_person(Person *me)
{
    printf("Person: %s,\n\
        Rank: %d,\n\
        Work Level: %d\n", me->name, me->rank, me->work_level);
}

int going_to_work(Person *me)
{
    printf("%s going to work\n", me->name);
    return me->work_level;
}

int going_home(Person *me)
{
    printf("%s going home\n", me->name);
    return me->work_level;
}

void browsing_phone(Person *me)
{
    printf("%s I'm browsing my phone\n", me->name);
    sleep(1);
}

void request_elevator(Person *me, int from, int to, int elevator_rank)
{
    printf("%s is requesting the elevator...\n", me->name);;
}

void shutdown_the_elevator(Person *me, int elevator_rank)
{
    ElevatorData transfer_data = ElevatorData_default;
    /* int MPI_Send(const void *buf, int count, MPI_Datatype datatype,
            int dest, int tag, MPI_Comm comm) */
    int tag = 1;
    printf("%s is shutting down the elevator\n", me->name);
    MPI_Send(&transfer_data.shutdown_signal, 3, MPI_INT, elevator_rank, tag, MPI_COMM_WORLD);
}
/* Person.c Ends */

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

            printf("Everybody working... working...\n");
            sleep(2);
            printf("Two second pause done\n");
        }

        if (rank != elevator_rank)
        {
            level = going_home(&me);
            request_elevator(&me, level, 0, elevator_rank);
        }
        
        MPI_Barrier(group_comm);
        sleep(5);
        printf("Everyone done working\n");
        if (group_rank == 0)
        {
            shutdown_the_elevator(&me, elevator_rank);
            printf("The office building is closed\n");
        }
    }

    MPI_Comm_free(&group_comm);
    MPI_Finalize();
    return 0;
}
