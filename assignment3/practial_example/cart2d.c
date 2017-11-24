#include <stdio.h>
#include <mpi.h>
/*Constants for numbers*/
#define SIZE 16
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

int main (int argc, char** argv) {
	int numtasks, rank, source, dest, outbuf, i, tag=1;
	/* MPI_PROC_NULL indicates a 'rank' for a so-called 'dummy process' */
	int inbuf[4] = {MPI_PROC_NULL, MPI_PROC_NULL, MPI_PROC_NULL, MPI_PROC_NULL}; /* Prepares variables to be used in async communication*/
	int nbrs[4];
	int dims[2] = {4, 4}, periods[2] = {1,1}, reorder = 1;
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

	printf("rank = %d, coords = %d %d having neighbours (u, d, l, r) = %d %d %d %d \n", rank, coords[0], coords[1], nbrs[UP], nbrs[DOWN], nbrs[LEFT], nbrs[RIGHT]);

	/*Do some work with MPI communication operations...
	  e.g. exchanging simple data with all neighbours*/
	outbuf = rank;
	for(i = 0; i<4; i++){
		dest=nbrs[i];
		source=nbrs[i];

		/*Perform non-blocking communication*/
		MPI_Isend(&outbuf, 1, MPI_INT, dest, tag, MPI_COMM_WORLD, &reqs[i]);
		MPI_Irecv(&inbuf[i], 1, MPI_INT, source, tag, MPI_COMM_WORLD, &reqs[i+4]); /*4 is kind of an offset*/
	}

	/*Wait fo rgiven MPI Requests to complete*/
	MPI_Waitall(8, reqs, stats);
	printf("rank = %d, has received (u,d,l,r,) = %d %d %d %d \n", rank, inbuf[UP], inbuf[DOWN], inbuf[LEFT], inbuf[RIGHT]);

	MPI_Finalize();

	return 0;
}