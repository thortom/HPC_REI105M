#include <stdio.h>
#include <mpi.h>
#include <time.h>
#include "logger.h"

/*Constants for numbers*/
#define SIZE 16
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3
#define MAX_NUMB_FISH 4

/* fish.h */
typedef struct fish_group_identity {
    int group_number;
    int numb_fish;
    int direction;
} fish_group;

/* Global variables */
fish_group FISH_NULL_DATA = {-1, -1};

void fish_group_constructor(fish_group *me, int group_number);
void print_fish_group(fish_group *me);
void update_fish_direction(fish_group *me);
int fish_data_equal(fish_group data_1, fish_group data_2);
/* fish.h - Ends */
/* fish.c */
void fish_group_constructor(fish_group *me, int group_number)
{
    me->group_number = group_number;
    /* random int between 10 and 20 */
    me->numb_fish = (rand() % 11) + 10;
    /* Up, Down, Left, Right */
    me->direction = (rand() % 4);
}

void print_fish_group(fish_group *me)
{
    log_info("Fish group number: %d, contains: %d fish", me->group_number, me->numb_fish);
}

void update_fish_direction(fish_group *me)
{
    /* Up, Down, Left, Right */
    me->direction = rand() % 4;
    log_debug("Fish group: %d heading: %d", me->group_number, me->direction);
}

int fish_data_equal(fish_group data_1, fish_group data_2)
{
    if (data_1.group_number == data_2.group_number &&
        data_1.numb_fish == data_2.numb_fish)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
/* fish.c - Ends */
/* boat.h */
typedef struct boat_identity {
    int number;
    int numb_fish_caught;
    int direction;
} boat;

/* Global variables */
boat BOAT_NULL_DATA = {-1, -1};

void boat_constructor(boat *me, int number);
void print_boat(boat *me);
void update_boat_direction(boat *me);
int boat_data_equal(boat data_1, boat data_2);
/* boat.h - Ends */
/* boat.c */
void boat_constructor(boat *me, int number)
{
    me->number = number;
    me->numb_fish_caught = 0;
    /* Up, Down, Left, Right */
    me->direction = (rand() % 4);
}

void print_boat(boat *me)
{
    log_info("Boat number: %d, has caught: %d fish", me->number, me->numb_fish_caught);
}

void update_boat_direction(boat *me)
{
    /* Up, Down, Left, Right */
    me->direction = rand() % 4;
    log_debug("Boat: %d heading: %d", me->number, me->direction);
}

int boat_data_equal(boat data_1, boat data_2)
{
    if (data_1.number == data_2.number &&
        data_1.numb_fish_caught == data_2.numb_fish_caught)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
/* boat.c - Ends */

/* node.h */
typedef struct node_identity {
    int rank;
    int coords[2];
    boat my_boat;
    fish_group my_fish[MAX_NUMB_FISH];
} Node;
/* node.h - Ends */
/* node.c */
void node_constructor(Node *me, int rank, int coords[])
{
    me->rank = rank;
    me->coords[0] = coords[0];
    me->coords[1] = coords[1];

    int i = 0;
    boat my_boat = BOAT_NULL_DATA;
    for (i = 0; i < MAX_NUMB_FISH; i++)
    {
        me->my_fish[i] = FISH_NULL_DATA;
    }
}

void initialize_node_data(Node *me) /*int rank, int coords[], fish_group my_fish[], boat *my_boat*/
{
    if (me->coords[0] == 0 && me->coords[1] == 0)
    {
        boat_constructor(&me->my_boat, me->rank);
    }
    else if ((me->coords[0] == 1 && me->coords[1] == 1) ||
        (me->coords[0] == 0 && me->coords[1] == 1))
    {
        fish_group fish;
        fish_group_constructor(&fish, me->rank);
        me->my_fish[0] = fish;
        log_debug("Coords (%d, %d) contains fish", me->coords[0], me->coords[1]);
        print_fish_group(&fish);
    }
}

void update_status(Node *node)
{
    int i = 0;
    for (i; i < MAX_NUMB_FISH; i++)
    {
        if (!fish_data_equal(node->my_fish[i], FISH_NULL_DATA))
        {
            update_fish_direction(&(node->my_fish[i]));
        }
    }
    if (!boat_data_equal(node->my_boat, BOAT_NULL_DATA))
    {
        update_boat_direction(&(node->my_boat));
    }
}

void get_number_of_items_to_transmit(Node *node, int n_items_to_transmit[], int send_to)
{
    int i = 0;
    for (i; i < MAX_NUMB_FISH; i++)
    {
        if (!fish_data_equal(node->my_fish[i], FISH_NULL_DATA))
        {
            if (node->my_fish[i].direction == send_to)
            {
                n_items_to_transmit[0] = n_items_to_transmit[0] + 1;
            }
        }
    }
    if (!boat_data_equal(node->my_boat, BOAT_NULL_DATA))
    {
        if (node->my_boat.direction == send_to)
        {
            n_items_to_transmit[1] = n_items_to_transmit[1] + 1;
        }
    }
}
/* node.c - Ends */

int main (int argc, char** argv)
{
    init_logger();
    int numtasks, rank, source, dest, outbuf, i, j, tag=1, joining_nodes=4;
    /* MPI_PROC_NULL indicates a 'rank' for a so-called 'dummy process' */
    int inbuf[4] = {MPI_PROC_NULL, MPI_PROC_NULL, MPI_PROC_NULL, MPI_PROC_NULL}; /* Prepares variables to be used in async communication*/
    int nbrs[4];
    int dims[2] = {2, 2}, periods[2] = {1,1}, reorder = 1;
    int coords[2];
    MPI_Comm cartcomm;

    MPI_Request reqs[8];
    MPI_Status stats[8];

    /* Starting with MPI program*/
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    /* Communicator, number of dimensions, int array specifying the number of processes in each dimension,
       logical array of size of dimensions specifying if grid is periodic or not in each dimension,
       can reorder or not, communicator with new cartesian topology*/
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, reorder, &cartcomm); 

    MPI_Comm_rank(cartcomm, &rank);

    MPI_Cart_coords(cartcomm, rank, 2, coords);

    MPI_Cart_shift(cartcomm, 0, 1, &nbrs[UP], &nbrs[DOWN]);
    MPI_Cart_shift(cartcomm, 1, 1, &nbrs[LEFT], &nbrs[RIGHT]);

    /* Putting time as seed for the random generator */
    srand(time(NULL) + rank);

    int is_ocean;
    /* Coord (0, 0) is the harbor */
    if(coords[0] == 0 && coords[1] == 0)
    { 
        is_ocean = 0;
        log_debug("Rank = %d, is the harbor", rank);
    }
    else
    {
        is_ocean = 1;
        log_debug("Rank = %d, is ocean", rank);
    }

    /*sleep(1);*/

    log_debug("Rank = %d, coord = (%d %d) my neighbors ranks are: (u, d, l, r) = (%d, %d, %d, %d)", rank, coords[0], coords[1], nbrs[UP], nbrs[DOWN], nbrs[LEFT], nbrs[RIGHT]);

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

    sleep(1);

    /*Wait forgiven MPI Requests to complete*/
    MPI_Waitall(8, reqs, stats);
    log_debug("rank = %d, has joining sea: (u, d, l, r) = (%d, %d, %d, %d)", rank, inbuf[UP], inbuf[DOWN], inbuf[LEFT], inbuf[RIGHT]);

    sleep(1);

    /*************************************************************/
    /* Here start the main stuff (maybe put in another function) */
    /*************************************************************/
    Node node;
    node_constructor(&node, rank, coords);

    /* Adding fish and boats to each node */
    initialize_node_data(&node);

    /* Update fish and boats status (update there desired directions) */
    update_status(&node);

    sleep(1);

    int n_items_to_transmit[2];
    int n_items_receiving[4][2];
    int running = 3;
    while (running)
    {
        /* For each neighbor check if fish/boat are coming to me (Isend and Irecv)*/
        for(i = 0; i < joining_nodes; i++)
        {
            n_items_to_transmit[0] = 0;
            n_items_to_transmit[1] = 0;
            get_number_of_items_to_transmit(&node, n_items_to_transmit, i);

            dest = nbrs[i];
            source = nbrs[i];

            MPI_Isend(&n_items_to_transmit, 2, MPI_INT, dest, tag, MPI_COMM_WORLD, &reqs[i]);
            log_debug("Node rank: %d will transmit %d fish groups and %d boats", rank, n_items_to_transmit[0],
                                                                      n_items_to_transmit[1]);
            n_items_receiving[i][0] = 0;
            n_items_receiving[i][1] = 0;
            MPI_Irecv(&n_items_receiving[i], 2, MPI_INT, source, tag, MPI_COMM_WORLD, &reqs[i + joining_nodes]);
            log_debug("Node rank: %d will receive %d fish groups and %d boats", rank, n_items_receiving[i][0],
                                                                      n_items_receiving[i][1]);
        }

        /* Move the boats and fish to the next node */
        /* TODO */

        /* Boats go fish */
        /* TODO */

        /* Update fish and boats status */
        update_status(&node);

        sleep(1);
        running -= 1;
    }

    MPI_Finalize();
    return 0;
}
