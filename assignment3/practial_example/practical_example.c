#include <stdio.h>
#include <mpi.h>

int main (int argc, char** argv) {
	int rank, size;

	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	int dims[2] = {3, 4};
	int periods[2] = {1,1};
	int coords[2];
	int reorder = 1;

	int source, dest, a, b;

	MPI_Comm comm_2d;
	MPI_Status status;

	MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, reorder, &comm_2d);

	MPI_Cart_coords(comm_2d, rank, 2, coords);

	MPI_Cart_shift(comm_2d, 0, 1, &source, &dest);

	a = rank; b = 100;

	MPI_Sendrecv(&a, 1, MPI_INT, dest, 13, &b, 1, MPI_INT, source, 13, comm_2d, &status);

	printf("rank %d, source is %d\n", rank, source);
	printf("rank %d, dest is %d\n", rank, dest);
	printf("rank %d coordinates are %d %d\n", rank, coords[0], coords[1]);

	printf("rank %d send to dest = %d the value %d \n", rank, dest, a);
	printf("rank %d received from source = %d, the value %d \n", rank, source, b);
	printf("-------\n");

	MPI_Finalize();
	return 0;
}
