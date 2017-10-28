#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <unistd.h>

/* logger.h */
/* Global start time */
double START_TIME;

void init_logger();
double get_current_time();

/* End logger.h */

/* logger.c */
/* log_info macro */
#define log_info(format, ...) printf("%f sec [%s:%d] " format "\n", (get_current_time() - START_TIME), __FILE__, __LINE__, ## __VA_ARGS__)
#define log_debug(format, ...) log_info("DEBUG\n" format "")

void init_logger()
{
    /* Setting the start time for this program */
    START_TIME = get_current_time();
}

double get_current_time()
{
    return MPI_Wtime();
}

/* End logger.c */

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
void Elevator_constructor(Elevator *me, int rank, int numb_persons);
void print_elevator_data(ElevatorData data);
void print_elevator(Elevator *me);
void start(Elevator *me);

/* Elevator.h Ends */

/* Elevator.c */
void init_elevator_data()
{
    /* create a type for struct ElevatorData */
    const int nitems=3;
    int          blocklengths[3] = {1, 1, 1};
    MPI_Datatype types[3] = {MPI_INT, MPI_INT, MPI_INT};
    MPI_Aint     offsets[3];

    offsets[0] = offsetof(ElevatorData, person_rank);
    offsets[1] = offsetof(ElevatorData, from);
    offsets[2] = offsetof(ElevatorData, to);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_elevator_data_type);
    MPI_Type_commit(&mpi_elevator_data_type);

    ELEVATOR_NULL_DATA.person_rank = -999;
    ELEVATOR_NULL_DATA.from = -999;
    ELEVATOR_NULL_DATA.to = -999;
    ELEVATOR_SHUTDOWN_SIGNAL.person_rank = -1;
    ELEVATOR_SHUTDOWN_SIGNAL.from = -1;
    ELEVATOR_SHUTDOWN_SIGNAL.to = -1;
}

void elevator_data_free()
{
    MPI_Type_free(&mpi_elevator_data_type);
}

int elevator_data_equal(ElevatorData data_1, ElevatorData data_2)
{
    if (data_1.person_rank == data_2.person_rank &&
        data_1.from == data_2.from &&
        data_1.to == data_2.to)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void Elevator_constructor(Elevator *me, int rank, int numb_persons)
{
    me->rank = rank;
    me->current_level = 0;
    me->is_running = 1;
    int i;
    for (i = 0; i < numb_persons; i++)
        me->persons_allowed[i] = i;
    me->numb_persons = numb_persons;
}

void print_elevator_data(ElevatorData data)
{
    log_info("person_rank: %d, from: %d, to: %d", data.person_rank, data.from, data.to);
}

void print_elevator(Elevator *me)
{
    log_info("Elevator:, Rank: %d, Current Level: %d", me->rank, me->current_level);
}

void start(Elevator *me)
{
    MPI_Status status;
    int i, j, n, tag = 1, flag = 0;
    int travel_time, one_level_travel_time = 1;
    ElevatorData data = ELEVATOR_NULL_DATA;

    ElevatorData persons_to_transfer[2] = {ELEVATOR_NULL_DATA, ELEVATOR_NULL_DATA};

    int numb_requests = 0;
    ElevatorData *incomming_requests = (ElevatorData*) malloc(me->numb_persons * sizeof(ElevatorData));
    for (i = 0; i <  me->numb_persons; i++)
    {
        *(incomming_requests + i) = ELEVATOR_NULL_DATA;
    }  

    while (me->is_running)
    {
        /* Check if any person is requesting the elevator */
        for (i = 1; i <= me->numb_persons; i++)
        {
            MPI_Iprobe(i, tag, MPI_COMM_WORLD, &flag, &status);

            if (flag == 1)
            {
                MPI_Recv(&data, 1, mpi_elevator_data_type, i, tag, MPI_COMM_WORLD, &status);

                if (elevator_data_equal(data, ELEVATOR_SHUTDOWN_SIGNAL))
                {
                    me->is_running = 0;
                    log_info("The elevator is shutting down");
                    break;
                }
                else if (!elevator_data_equal(data, ELEVATOR_NULL_DATA))
                {
                    /* TODO: Ensure that the ELEVATOR_NULL_DATA does not change here */
                    /* Collect all passenger requests here*/
                    log_info("Collecting passenger request for rank %d", data.person_rank);
                    (*(incomming_requests + (i - 1))).person_rank = data.person_rank;
                    (*(incomming_requests + (i - 1))).from = data.from;
                    (*(incomming_requests + (i - 1))).to = data.to;
                    numb_requests = numb_requests + 1;
                }
            }
        }

        /* Process the requests */
        while (numb_requests)
        {
            for (i = 0; i < me->numb_persons; i++)
            {
                if (!elevator_data_equal(*(incomming_requests + i), ELEVATOR_NULL_DATA))
                {
                    if (elevator_data_equal(persons_to_transfer[0], ELEVATOR_NULL_DATA))
                    {
                        (persons_to_transfer[0]).person_rank = (*(incomming_requests + i)).person_rank;
                        (persons_to_transfer[0]).from = (*(incomming_requests + i)).from;
                        (persons_to_transfer[0]).to = (*(incomming_requests + i)).to;

                        *(incomming_requests + i) = ELEVATOR_NULL_DATA;
                        numb_requests = numb_requests - 1;

                        log_info("person to transfer");
                        print_elevator_data(persons_to_transfer[0]);
                    }
                    else if (elevator_data_equal(persons_to_transfer[1], ELEVATOR_NULL_DATA) &&
                                ((*(incomming_requests + i)).from == persons_to_transfer[0].from))
                    {
                        (persons_to_transfer[1]).person_rank = (*(incomming_requests + i)).person_rank;
                        (persons_to_transfer[1]).from = (*(incomming_requests + i)).from;
                        (persons_to_transfer[1]).to = (*(incomming_requests + i)).to;

                        *(incomming_requests + i) = ELEVATOR_NULL_DATA;
                        numb_requests = numb_requests - 1;

                        log_info("second person to transfer");
                        print_elevator_data(persons_to_transfer[1]);
                    }
                }

                /* Two persons to collect or only one to collect on the specific level */
                if (!elevator_data_equal(persons_to_transfer[1], ELEVATOR_NULL_DATA) || (i == (me->numb_persons - 1)))
                {
                    if (!elevator_data_equal(persons_to_transfer[1], ELEVATOR_NULL_DATA))
                    {
                        log_info("Picking up two persons");
                        print_elevator_data(persons_to_transfer[0]);
                        log_info("    and");
                        print_elevator_data(persons_to_transfer[1]);
                        n = 2;
                    }
                    else if (!elevator_data_equal(persons_to_transfer[0], ELEVATOR_NULL_DATA))
                    {
                        log_info("Picking up one person");
                        print_elevator_data(persons_to_transfer[0]);
                        n = 1;
                    }
                    for (j = 0; j < n; j++)
                    {
                        log_debug("j=%d, n=%d", j, n);
                        log_info("Sending the travel_time to rank=%d", persons_to_transfer[j].person_rank);
                        travel_time = abs(persons_to_transfer[j].to - persons_to_transfer[j].from) * one_level_travel_time;
                        MPI_Send(&travel_time, 1, MPI_INT, persons_to_transfer[j].person_rank, tag, MPI_COMM_WORLD);
                        persons_to_transfer[j] = ELEVATOR_NULL_DATA;
                    }
                    n = 0;
                }
            }
        }
    }

    /* Releasing dynamically allocated memory */
    free(incomming_requests);
    elevator_data_free();
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

    /* TODO: Maybe move this init to elevator.c (only run once)*/
    /* Only one elevator instance allowed in program scope */
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

            log_info("Everybody working... working...");
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
