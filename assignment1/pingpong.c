#include <stdio.h>
#include <mpi.h>

int main (int argc, char *argv[])
{
    int numtasks, rank, dest, source, rc, count, tag=1;
    int inmsg, outmsg=12345;

    MPI_Status Stat;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
    {
        dest = 1; source = 1;
        rc = MPI_Send(&outmsg, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
        rc = MPI_Recv(&inmsg, 1, MPI_INT, source, tag, MPI_COMM_WORLD, &Stat);
    }
    else if (rank == 1)
    {
        dest = 0; source = 0;
        rc = MPI_Recv(&inmsg, 1, MPI_INT, source, tag, MPI_COMM_WORLD, &Stat);
        rc = MPI_Send(&outmsg, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
    }

    rc = MPI_Get_count(&Stat, MPI_INT, &count);

    printf("Task %d: Received {%d} as message from task %d with tag %d \n", rank, inmsg, Stat.MPI_SOURCE, Stat.MPI_TAG);
    
    MPI_Finalize();
    return 0;
}
