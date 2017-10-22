/* Time Stuff: https://blog.habets.se/2010/09/gettimeofday-should-never-be-used-to-measure-time.html */

#include <omp.h>
#include <stdio.h>
#include <time.h>

int time_elapsed(struct timespec end, struct timespec start)
{
    printf("start.sec: %d, nsec: %d\n", start.tv_sec, start.tv_nsec);
    printf("end.sec: %d, nsec: %d\n", end.tv_sec, end.tv_nsec);

    long nstart = (start.tv_sec * 1000 * 1000) + start.tv_nsec;
    long nend = (end.tv_sec * 1000 * 1000) + start.tv_nsec;
    
    printf("nstart: %d", nstart);
    printf("nend: %d", nend);
    printf("Warning: Integer division?");
    double sec = (nend - nstart) / (1000 * 1000);
    printf("sec: %d", sec);
    return sec;
}

int main(int argc, char *argv[])
{
    int bstart, bend, blen, numth, tid, i;
    int N = 10;
    int a[N], b[N], c[N];
    for (i = 0; i < N; i = i + 1)
    {
        b[i] = i;
        c[i] = i * i;
    }
    /* TODO: Move this over */
    struct timespec ts_start;
    struct timespec ts_end;

    clock_gettime(CLOCK_MONOTONIC, &ts_start);

    usleep(1100 * 1000);

    clock_gettime(CLOCK_MONOTONIC, &ts_end);
    /* is this t_time object? */
    printf("Time elapsed: %f sec and %f nsec",
                                (ts_end.tv_sec - ts_start.tv_sec),
                                (ts_end.tv_nsec - ts_start.tv_nsec));
    printf("Time elapsed: %f", time_elapsed(end, start));

    /**/

    printf("Setting number of threads\n");
    omp_set_num_threads(2);
    #pragma omp parallel private(bstart, bend, blen, numth, tid, i)
    {
        numth = omp_get_num_threads();
        tid = omp_get_thread_num();

        printf("Hello World from thread = %d\n", tid);

        if (tid == 0)
        {
            printf("Number of threads in parallel region = %d\n", numth);
        }

        printf("numth: %d\n", numth);
        printf("N .. numth: %d\n", N % numth);

        blen = N / numth;
        if (tid < (N % numth))
        {
            blen = blen + 1;
            bstart = blen * tid;
        }
        else
        {
            bstart = blen * tid + (N % numth);
        }
        bend = bstart + blen - 1;

        printf("bstart:%d, bend:%d\n", bstart, bend);
        for (i = bstart; i < bend; i = i + 1)
        {
            a[i] = b[i] + c[i];
            printf("a[%d]=%d\n", i, a[i]);
        }
    }

    printf("The final array is:\n");
    for (i = 0; i < N; i = i + 1)
    {
        printf("a[%d]=%d\n", i, a[i]);
    }
}
