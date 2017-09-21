#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

int sum_array(int array[], int num_elements)
{
   int i, sum = 0;
   for (i = 0; i < num_elements; i++)
   {
     sum = sum + array[i];
   }

   return sum;
}

int main (int argc, char *argv[])
{
    int world_rank, world_size;
    int source, count;
    int elements_per_proc = 4;
    int array[elements_per_proc * world_size];
    int sub_array[elements_per_proc];

    MPI_Status status;
    MPI_Request request;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    source = 0;
    count = world_size * elements_per_proc;


    if (world_rank == source)
    {
        for (int i = 0; i < count; i++)
            array[i] = i;
    }

    /* Scatter the random numbers to all processes */
    MPI_Scatter(array, elements_per_proc, MPI_INT, sub_array,
                elements_per_proc, MPI_INT, 0, MPI_COMM_WORLD);

    /* Compute the sum of your subset */
    int sub_sum = sum_array(sub_array, elements_per_proc);

    /* Gather all partial sums down to the root process */
    int sub_sums[world_size * elements_per_proc];
    MPI_Gather(&sub_sum, 1, MPI_INT, sub_sums, 1, MPI_INT, 0,
               MPI_COMM_WORLD);

    /* Compute the total sum of all numbers */
    if (world_rank == 0)
    {
        int sum = sum_array(sub_sums, world_size);
        printf("Node %d collected the total sum of %d\n", world_rank, sum);
    }
    else
    {
        printf("Node %d done\n", world_rank);
    }
 
    MPI_Finalize();
    return 0;
}
