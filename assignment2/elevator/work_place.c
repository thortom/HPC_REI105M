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

/* TODO: Fix this function it's flakey ... */
void start(Elevator *me)
{
    MPI_Status status;
    int i, j, tag = 1, flag = 0;
    int diff, travel_time, one_level_travel_time = 1;
    ElevatorData transfer_data = ElevatorData_default;
    int data[transfer_data.size];
    for (i = 0; i < transfer_data.size; i++)
    {
        data[i] = transfer_data.null_data[i];
    }
    int persons_to_transfer[6] = {0, 0, 0, 0, 0, 0};

    int numb_requests = 0;
    int *incomming_requests = (int*) malloc(me->numb_persons * transfer_data.size * sizeof(int));
    for (i = 0; i <  me->numb_persons; i++)
    {
        for (j = 0; j < transfer_data.size; j++)
        {
            *(incomming_requests + (i * transfer_data.size) + j) = 0;
        }
    }

    while (me->is_running)
    {
        /* Check if any person is requesting the elevator */
        for (i = 1; i <= me->numb_persons; i++)
        {
            MPI_Iprobe(i, tag, MPI_COMM_WORLD, &flag, &status);

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
                else if (memcmp(data, transfer_data.null_data, transfer_data.size) != 0)
                {
                    /* Collect all passenger requests here*/
                    printf("Collecting passenger request for rank %d\n", data[transfer_data.person_rank]);
                    *(incomming_requests + ((i - 1) * transfer_data.size) + transfer_data.person_rank) = data[transfer_data.person_rank];
                    *(incomming_requests + ((i - 1) * transfer_data.size) + transfer_data.from) = data[transfer_data.from];
                    *(incomming_requests + ((i - 1) * transfer_data.size) + transfer_data.to) = data[transfer_data.to];
                    numb_requests = numb_requests + 1;
                }
            }
        }

        /* Process the requests */
        while (numb_requests)
        {
            for (i = 0; i < me->numb_persons; i++)
            {
                /* No person is rank 0 */
                if (*(incomming_requests + (i * transfer_data.size) + transfer_data.person_rank))
                {
                    if (persons_to_transfer[0] == 0)
                    {
                        persons_to_transfer[transfer_data.person_rank] = \
                            *(incomming_requests + (i * transfer_data.size) + transfer_data.person_rank);
                        persons_to_transfer[transfer_data.from] = \
                            *(incomming_requests + (i * transfer_data.size) + transfer_data.from);
                        persons_to_transfer[transfer_data.to] = \
                            *(incomming_requests + (i * transfer_data.size) + transfer_data.to);

                        *(incomming_requests + (i * transfer_data.size) + transfer_data.person_rank) = 0;
                        numb_requests = numb_requests - 1;
                        printf("person to transfer=%d from=%d, to=%d\n", persons_to_transfer[transfer_data.person_rank],
                                                    persons_to_transfer[transfer_data.from], persons_to_transfer[transfer_data.to]);
                    }
                    else if ((persons_to_transfer[1 * transfer_data.size] == 0) && (*(incomming_requests + (i * transfer_data.size) + transfer_data.from) == persons_to_transfer[transfer_data.from]))
                    {
                        persons_to_transfer[transfer_data.size + transfer_data.person_rank] = \
                            *(incomming_requests + (i * transfer_data.size) + transfer_data.person_rank);
                        persons_to_transfer[transfer_data.size + transfer_data.from] = \
                            *(incomming_requests + (i * transfer_data.size) + transfer_data.from);
                        persons_to_transfer[transfer_data.size + transfer_data.to] = \
                            *(incomming_requests + (i * transfer_data.size) + transfer_data.to);

                        *(incomming_requests + (i * transfer_data.size) + transfer_data.person_rank) = 0;
                        numb_requests = numb_requests - 1;
                    }

                    /* Two persons to collect or only one to collect on the specific level */
                    printf("persons_to_transfer[(1 * transfer_data.size) + transfer_data.person_rank] ->%d\ni -> %d (me->numb_persons - 1) ->%d\n",
                                persons_to_transfer[(1 * transfer_data.size) + transfer_data.person_rank], i, (me->numb_persons - 1));
                    if ((persons_to_transfer[(1 * transfer_data.size) + transfer_data.person_rank] != 0) || (i == (me->numb_persons - 1)))
                    {
                        if (persons_to_transfer[(1 * transfer_data.size) + transfer_data.person_rank] != 0)
                        {
                            printf("Picking up persons of rank %d and %d on level=%d\n", persons_to_transfer[transfer_data.person_rank],
                                                                            persons_to_transfer[(1 * transfer_data.size) + transfer_data.person_rank],
                                                                            persons_to_transfer[transfer_data.from]);
                        }
                        else
                        {
                            printf("Picking up person of rank %d on level=%d\n", persons_to_transfer[transfer_data.person_rank], persons_to_transfer[transfer_data.from]);
                        }
                        for (i = 0; i < 2; i++)
                        {
                            if (persons_to_transfer[(i * transfer_data.size) + transfer_data.person_rank])
                            {
                                printf("Sending the travel_time to rank=%d\n", persons_to_transfer[(i * transfer_data.size) + transfer_data.person_rank]);
                                travel_time = abs(persons_to_transfer[transfer_data.to] - persons_to_transfer[transfer_data.from]) * one_level_travel_time;
                                MPI_Send(&travel_time, 1, MPI_INT, persons_to_transfer[(i * transfer_data.size) + transfer_data.person_rank], tag, MPI_COMM_WORLD);
                            }
                        }
                        for (j = 0; j < (2 * transfer_data.size); j++)
                        {
                            persons_to_transfer[j] = 0;
                        }
                    }
                }
            }
        }
    }

    /* Releasing dynamically allocated memory */
    free(incomming_requests);
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
    printf("%s is requesting the elevator...\n", me->name);

    MPI_Status status;
    MPI_Request request;
    int tag = 1;

    ElevatorData transfer_data = ElevatorData_default;
    int data[transfer_data.size];
    data[transfer_data.person_rank] = me->rank;
    data[transfer_data.from] = from;
    data[transfer_data.to] = to;

    /* Send the transfer data to the elevator */
    MPI_Isend(&data, transfer_data.size, MPI_INT, elevator_rank, tag, MPI_COMM_WORLD, &request);

    int flag = 0;
    while (!flag)
    {
        /* Check regularly whether the elevator is coming */
        MPI_Test(&request, &flag, &status);
        browsing_phone(me);
    }
    printf("%s the elevator is coming soon for me\n", me->name);

    int travel_time;
    MPI_Irecv(&travel_time, 1, MPI_INT, elevator_rank, tag, MPI_COMM_WORLD, &request);

    flag = 0;
    while (!flag)
    {
        MPI_Test(&request, &flag, &status);
        browsing_phone(me);
    }

    printf("%s the elevator has arrived for me\n", me->name);
    sleep(travel_time);
    printf("%s arrived at my level\n", me->name);
}

void shutdown_the_elevator(Person *me, int elevator_rank)
{
    ElevatorData transfer_data = ElevatorData_default;
    int tag = 1;
    printf("%s is shutting down the elevator\n", me->name);
    MPI_Send(&transfer_data.shutdown_signal, 3, MPI_INT, elevator_rank, tag, MPI_COMM_WORLD);
}
/* Person.c Ends */

/* Global start time */
double START_TIME;

/* log_info macro */
#define log_info(format, ...) printf("%f sec [%s:%d] " format "\n", (MPI_Wtime() - START_TIME), __FILE__, __LINE__, ## __VA_ARGS__)

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
    /* Setting the global start time */
    START_TIME = MPI_Wtime();
    log_info("Logging stuff");
    return 0;

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
            sleep(10);
        }

        if (rank != elevator_rank)
        {
            level = going_home(&me);
            request_elevator(&me, level, 0, elevator_rank);
        }
        
        printf("%s is done working\n", me.name);
        MPI_Barrier(group_comm);
        sleep(5);
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
