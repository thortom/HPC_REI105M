/* http://www.st.ewi.tudelft.nl/~varbanescu/ASCI_A24.2k12/ASCI_A24_Day2_Part2_OpenMP.pdf */
#include <omp.h>
#include <stdio.h>
#include <time.h>

void log(double time_start, double time_now, char* msg)
{
    /* TODO: This... */
    printf("{}: %s\n", msg);
}

int main(int argc, char *argv[])
{
    int bstart, bend, blen, numth, tid, i;
    int N = 10;
    double a[N], b[N], c[N];
    for (i = 0; i <= N; i = i + 1)
    {
        b[i] = i;
        c[i] = i * i;
    }

    double time_start;
    time_start = clock()           /* TODO: Maybe use MPI_Wtime() */
    
    omp_set_num_threads(2);
    #pragma omp parallel private(bstart, bend, blen, numth, tid, i)
    {
        numth = omp_get_num_threads();
        tid = omp_get_thread_num();

        printf("%s: Hello World from thread = %d\n", clock() - time_start, tid);

        if (tid == 0)
        {
            printf("%s: Number of threads in parallel region = %d\n",
                                            clock() - time_start, numth);
        }

        blen = N / numth;
        if (tid < N % numth)
        {
            blen = blen + 1;
            bstart = blen * tid + 1;
        }
        else
        {
            bstart = blen * tid + (N % numth) + 1;
        }
        blend = bstart + blen - 1;

        for (i = bstart; i <= bend; i = i + 1)
        {
            a[i] = b[i] + c[i];
        }
    }

    printf("%s: The final array is:\n", clock() - time_start);
    for (i = 0; i <= N; i = i + 1)
    {
        printf("%s: a[%d] = %d\n", clock() - time_start, i, a[i]);
    }
} 