#include <stdio.h>
#include <mpi.h>
#include <math.h>
#include <time.h>
#include "logger.h"
#include "physics.h"
#include "fish.h"
#include "boat.h"

/* Constant variables */
#define MAX_NUMB_FISH 7
const Fish_group FISH_NULL_DATA = {-1, -1, -1};
const Boat       BOAT_NULL_DATA = {-1, -1, -1};

/* Global variables */
MPI_Datatype mpi_fish_data_type;
MPI_Datatype mpi_boat_data_type;
MPI_Datatype mpi_transmit_data_type;

typedef struct node_identity {
    int rank;
    int coords[2];
    Boat my_boat;
    Fish_group my_fish[MAX_NUMB_FISH];
} Node;

typedef struct transmit_data_identity {
    int numb_fish;
    int numb_boats;
    int from_rank;
} Transmit_data;

/* Global variables */
Transmit_data TRANSMIT_NULL_DATA = {0, 0, -1};

void node_constructor(Node *me, int rank, int coords[])
{
    me->rank = rank;
    me->coords[0] = coords[0];
    me->coords[1] = coords[1];

    me->my_boat = BOAT_NULL_DATA;
    for (int i = 0; i < MAX_NUMB_FISH; i++)
    {
        me->my_fish[i] = FISH_NULL_DATA;
    }
}

void initialize_node_data(Node *me, int world_size)
{
    if (me->coords[0] == 0 && me->coords[1] == 0)
    {
        boat_constructor(&me->my_boat, me->rank);
    }
    /* Add the second boat if the world is big enough */
    else if (world_size > 4 &&
             me->coords[0] == 0 && me->coords[1] == 1)
    {
        /* TODO: Should this boat start from (0, 0) or is it OK here?*/
        boat_constructor(&me->my_boat, me->rank);
    }
    /* Assign fish to almost half of the cells */
    else if (((me->coords[0] + me->coords[1]) % 2) == 0)
    {
        Fish_group fish;
        fish_group_constructor(&fish, me->rank);
        me->my_fish[0] = fish;
        log_debug("Node [%d]: (%d, %d) contains fish", me->rank, me->coords[0], me->coords[1]);
        print_fish_group(&fish);
    }
}

void update_status(Node *me)
{
    int i = 0;
    for (i; i < MAX_NUMB_FISH; i++)
    {
        if (!fish_data_equal(me->my_fish[i], FISH_NULL_DATA))
        {
            update_fish_direction(&(me->my_fish[i]));
        }
    }
    if (!boat_data_equal(me->my_boat, BOAT_NULL_DATA))
    {
        update_boat_direction(&(me->my_boat), me->coords);
    }
}

void get_number_of_items_to_transmit(Node *node, Transmit_data *data_out, int send_to)
{
    data_out->from_rank = node->rank;
    for (int i = 0; i < MAX_NUMB_FISH; i++)
    {
        if (!fish_data_equal(node->my_fish[i], FISH_NULL_DATA))
        {
            if (node->my_fish[i].direction == send_to)
            {
                data_out->numb_fish = data_out->numb_fish + 1;
            }
        }
    }
    if (!boat_data_equal(node->my_boat, BOAT_NULL_DATA))
    {
        if (node->my_boat.direction == send_to)
        {
            data_out->numb_boats = data_out->numb_boats + 1;
        }
    }
}

void add_fish(Node *node, Fish_group new_group)
{
    new_group.direction = -1;
    for (int i = 0; i < MAX_NUMB_FISH; i++)
    {
        if (fish_data_equal(node->my_fish[i], FISH_NULL_DATA))
        {
            node->my_fish[i] = new_group;
            return;
        }
    }
}

void log_node_info(Node *node)
{
    log_info("Node [%d] contains the following:", node->rank);
    for (int i = 0; i < MAX_NUMB_FISH; i++)
    {
        if (!fish_data_equal(node->my_fish[i], FISH_NULL_DATA))
        {
            log_info("      N[%d]: FishGroup [%d] containing %d fish",
                node->rank, node->my_fish[i].group_number, node->my_fish[i].numb_fish);
        }
    }
    if (!boat_data_equal(node->my_boat, BOAT_NULL_DATA))
    {
        log_info("      N[%d]: Boat [%d] containing %d fish",
                node->rank, node->my_boat.number, node->my_boat.numb_fish_caught);
    }
    log_info("Node [%d] end of info", node->rank);
}

void collect_transmit_info(Node *node, Transmit_data data_out[], Transmit_data data_in[], int nbrs[])
{
    int source, dest, i, tag=1, joining_nodes=4;
    MPI_Request reqs[8];
    MPI_Status stats[8];

    for(i = 0; i < joining_nodes; i++)
    {
        data_out[i] = TRANSMIT_NULL_DATA;
        get_number_of_items_to_transmit(node, &data_out[i], i);

        dest = nbrs[i];
        source = nbrs[i];

        MPI_Isend(&data_out[i], 1, mpi_transmit_data_type, dest, tag, MPI_COMM_WORLD, &reqs[i]);
        /* log_debug("Node rank: %d will transmit %d fish groups and %d boats, to %d",
                                node->rank, data_out[i].numb_fish, data_out[i].numb_boats, dest); */
        data_in[i] = TRANSMIT_NULL_DATA;
        MPI_Irecv(&data_in[i], 1, mpi_transmit_data_type, source, tag, MPI_COMM_WORLD, &reqs[i + joining_nodes]);
    }

    /* Wait forgiven MPI Requests to complete */
    MPI_Waitall(8, reqs, stats);
}

void transfer_data(Node *node, Transmit_data data_out[], Transmit_data data_in[], int nbrs[])
{
    int source, dest, i, j, k, ok, tag=1, joining_nodes=4;
    MPI_Request dummy_request;
    MPI_Status dummy_status;
    Fish_group new_fish;
    Boat dummy_boat;

    /* Transfer the fish first then the boats */
    /* If done in the same for-loop then the MPI_Send - MPI_Recv can interfere with each other and fail */
    for (i = 0; i < joining_nodes; i++)
    {
        /* log_debug("Node [%d] data_in: fish=%d, boats=%d, from=%d", node->rank, data_in[i].numb_fish,
                                    data_in[i].numb_boats, data_in[i].from_rank); */
        
        dest = nbrs[i];
        source = nbrs[i];
        
        for (j = 0; j < data_out[i].numb_fish; j++)
        {
            for (k = 0; k < MAX_NUMB_FISH; k++)
            {
                if (node->my_fish[k].direction == i)
                {
                    MPI_Isend(&node->my_fish[k], 1, mpi_fish_data_type, dest, tag, MPI_COMM_WORLD, &dummy_request);
                    node->my_fish[k] = FISH_NULL_DATA;
                }
            }
        }

        for (j = 0; j < data_in[i].numb_fish; j++)
        {
            MPI_Recv(&new_fish, 1, mpi_fish_data_type, source, tag, MPI_COMM_WORLD, &dummy_status);

            /* TODO: This can fail when there are more than MAX_NUMB_FISH fish groups in the ocean */
            add_fish(node, new_fish);
        }
    }

    /* Transfer the boats */
    for (i = 0; i < joining_nodes; i++)
    {
        dest = nbrs[i];
        source = nbrs[i];

        if (data_out[i].numb_boats)
        {
            MPI_Send(&node->my_boat, 1, mpi_boat_data_type, dest, tag, MPI_COMM_WORLD);
            log_debug("Node [%d] sent the new_boat", node->rank);
            MPI_Recv(&ok, 1, MPI_INT, source, tag, MPI_COMM_WORLD, &dummy_status);
            log_debug("Node [%d] received the OK signal", node->rank);
            /* ok is True if the boat did successfully transfer over */
            if (ok)
            {
                node->my_boat = BOAT_NULL_DATA;
            }
        }

        if (data_in[i].numb_boats)
        {
            if (boat_data_equal(node->my_boat, BOAT_NULL_DATA))
            {
                ok = 1;
                log_debug("Node [%d] will accept the boat from node [%d]", node->rank, source);
                MPI_Recv(&node->my_boat, 1, mpi_boat_data_type, source, tag, MPI_COMM_WORLD, &dummy_status);
                log_debug("Node [%d] received the new_boat", node->rank);
                MPI_Send(&ok, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
            }
            else
            {
                log_debug("Node [%d] already contains one boat thus it will not accept the boat from node [%d]",
                                                                                            node->rank, source);
                ok = 0; /* Not OK */
                /* The node already contains one boat thus it will not accept another one */
                MPI_Recv(&dummy_boat, 1, mpi_boat_data_type, source, tag, MPI_COMM_WORLD, &dummy_status);
                log_debug("Node [%d] received the dummy_boat", node->rank);
                MPI_Send(&ok, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
                log_debug("Node [%d] sent the not OK signal", node->rank);
            }
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
}

void do_action(Node *node)
{
    int fish_caught = 0;
    if (!boat_data_equal(node->my_boat, BOAT_NULL_DATA))
    {
        for (int i = 0; i < MAX_NUMB_FISH; i++)
        {
            if (!fish_data_equal(node->my_fish[i], FISH_NULL_DATA))
            {
                /* The boat catches some fish */
                fish_caught = rand() % node->my_fish[i].numb_fish;
                node->my_boat.numb_fish_caught += fish_caught;

                node->my_fish[i].numb_fish -= fish_caught;
                /* If all the fish in this group is caught then
                    the group it dead */
                if (node->my_fish[i].numb_fish == 0)
                {
                    node->my_fish[i] = FISH_NULL_DATA;
                }
            }
        }
    }
}

void init_mpi_custom_types()
{
    /* Create a MPI type for struct Fish_group */
    int nitems=3;
    int blocklengths[3] = {1, 1, 1};
    MPI_Datatype types[3] = {MPI_INT, MPI_INT, MPI_INT};
    MPI_Aint offsets[3];

    offsets[0] = offsetof(Fish_group, group_number);
    offsets[1] = offsetof(Fish_group, numb_fish);
    offsets[2] = offsetof(Fish_group, direction);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_fish_data_type);
    MPI_Type_commit(&mpi_fish_data_type);

    /* Create a MPI type for struct Boat */
    offsets[0] = offsetof(Boat, number);
    offsets[1] = offsetof(Boat, numb_fish_caught);
    offsets[2] = offsetof(Boat, direction);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_boat_data_type);
    MPI_Type_commit(&mpi_boat_data_type);

    /* Create a MPI type for struct Transmit_data*/
    offsets[0] = offsetof(Transmit_data, numb_fish);
    offsets[1] = offsetof(Transmit_data, numb_boats);
    offsets[2] = offsetof(Transmit_data, from_rank);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_transmit_data_type);
    MPI_Type_commit(&mpi_transmit_data_type);
}

void free_custom_mpi_types()
{
    MPI_Type_free(&mpi_fish_data_type);
    MPI_Type_free(&mpi_boat_data_type);
    MPI_Type_free(&mpi_transmit_data_type);
}

void init_cartesian_grid(int *rank, int coords[], int nbrs[], int dims[])
{
    int periods[2] = {1,1}, reorder = 1;
    int source, dest, outbuf, i, j, k, tag=1, joining_nodes=4;
    /* MPI_PROC_NULL indicates a 'rank' for a so-called 'dummy process' */
    int inbuf[4] = {MPI_PROC_NULL, MPI_PROC_NULL, MPI_PROC_NULL, MPI_PROC_NULL};
    MPI_Comm cartcomm;
    MPI_Request reqs[8];
    MPI_Status stats[8];

    /* Communicator, number of dimensions, int array specifying the number of processes in each dimension,
       logical array of size of dimensions specifying if grid is periodic or not in each dimension,
       can reorder or not, communicator with new cartesian topology */
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, reorder, &cartcomm); 

    MPI_Comm_rank(cartcomm, rank);

    MPI_Cart_coords(cartcomm, *rank, 2, coords);

    MPI_Cart_shift(cartcomm, 0, 1, &nbrs[UP], &nbrs[DOWN]);
    MPI_Cart_shift(cartcomm, 1, 1, &nbrs[LEFT], &nbrs[RIGHT]);

    /* Putting time as seed for the random generator */
    srand(time(NULL) + *rank);

    int is_ocean;
    /* Coordinate (0, 0) is the harbor */
    if(coords[0] == 0 && coords[1] == 0)
    { 
        is_ocean = 0;
        log_debug("Rank = %d, is the harbor", *rank);
    }
    else
    {
        is_ocean = 1;
        log_debug("Rank = %d, is ocean", *rank);
    }

    log_debug("Rank = %d, coord = (%d %d) my neighbors ranks are: (u, d, l, r) = (%d, %d, %d, %d)",
                        *rank, coords[0], coords[1], nbrs[UP], nbrs[DOWN], nbrs[LEFT], nbrs[RIGHT]);

    /*Do some work with MPI communication operations...
      e.g. exchanging simple data with all neighbors*/
    /*cell type is; 0: land, 1: water*/
    outbuf = is_ocean;
    for(i = 0; i < joining_nodes; i++)
    {
        dest=nbrs[i];
        source=nbrs[i];

        /*Perform non-blocking communication*/
        MPI_Isend(&outbuf, 1, MPI_INT, dest, tag, MPI_COMM_WORLD, &reqs[i]);
        MPI_Irecv(&inbuf[i], 1, MPI_INT, source, tag, MPI_COMM_WORLD, &reqs[i + joining_nodes]);
        /* inbuf contains whether the joining node is land or ocean */
    }

    /* Wait forgiven MPI Requests to complete */
    MPI_Waitall(8, reqs, stats);
    log_debug("rank = %d, has joining sea: (u, d, l, r) = (%d, %d, %d, %d)",
                        *rank, inbuf[UP], inbuf[DOWN], inbuf[LEFT], inbuf[RIGHT]);
}

int main (int argc, char** argv)
{
    /* TODO: This logger should be able to log to file with MPI-I/O */
    init_logger();

    MPI_File fh;
    int rank, world_size;
    int source, dest, outbuf, i, j, k, tag=1, joining_nodes=4;
    int nbrs[4];
    int dims[2];
    int coords[2];
    int l, offset;
    MPI_Request reqs[8], dummy_request;
    MPI_Status stats[8], dummy_status;

    /* Starting with MPI program*/
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    
    if (world_size == 4 || world_size == 9 || world_size == 16)
    {
        dims[0] = (int)sqrt((double)world_size);
        dims[1] = (int)sqrt((double)world_size);
    }
    else
    {
        log_info("The requested ocean size is not supported");
        return 1;
    }
    init_mpi_custom_types();
    
    init_cartesian_grid(&rank, coords, nbrs, dims);
    log_debug("Node rank: %d has coords: (%d, %d)", rank, coords[0], coords[1]);

    sleep(1);

    Node node;
    node_constructor(&node, rank, coords);

    /* Adding fish and boats to each node */
    initialize_node_data(&node, world_size);

    /* Update fish and boats status (update there desired directions) */
    update_status(&node);

    sleep(1);

    Transmit_data data_out[joining_nodes];
    Transmit_data data_in[joining_nodes];
    int running = 5;
    while (running)
    {
        /* For each neighbor check for where the fish and boat are going */
        collect_transmit_info(&node, data_out, data_in, nbrs);

        /* Exchange fish and boats to there correct destination node */
        transfer_data(&node, data_out, data_in, nbrs);

        log_node_info(&node);

        /* Boats go fish */
        do_action(&node);

        /* Update fish and boats status */
        update_status(&node);

        sleep(1);
        running -= 1;
        int buf[4];
        for(l=0; l < 4; l++){
            buf[l] = l
        }
        offset = rank*(4/world_size)*sizeof(int);
        MPI_File_open(MPI_COMM_WORLD, 'test', MPI_MODE_CREATE|MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);
        MPI_File_write_at(fhw, offset, buf, (4/size), MPI_INT, &status);
        MPI_File_close(&fh);
    }

    free_custom_mpi_types();
    MPI_Finalize();
    return 0;
}
