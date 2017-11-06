#include "elevator.h"

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
    int travel_time, max_travel_time = 0, one_level_travel_time = 2;
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
            sleep(1);

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
                        log_info("Sending the travel_time to rank=%d", persons_to_transfer[j].person_rank);
                        travel_time = abs(persons_to_transfer[j].to - persons_to_transfer[j].from) * one_level_travel_time;
                        MPI_Send(&travel_time, 1, MPI_INT, persons_to_transfer[j].person_rank, tag, MPI_COMM_WORLD);
                        persons_to_transfer[j] = ELEVATOR_NULL_DATA;
                        if (max_travel_time < travel_time)
                        {
                            max_travel_time = travel_time;
                        }
                    }
                    sleep(max_travel_time);
                    n = 0;
                }
            }
        }
    }

    /* Releasing dynamically allocated memory */
    free(incomming_requests);
    elevator_data_free();
}
