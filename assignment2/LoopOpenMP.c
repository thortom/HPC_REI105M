#include <omp.h>
#include <stdio.h>
#include <time.h>

/* Global start time */
struct timespec START_TIME;

/* TODO: Add this to logger.h */

double time_elapsed(void);

/* log_info macro */
#define log_info(format, ...) printf("%f sec [%s:%d] " format "\n", time_elapsed(), __FILE__, __LINE__, ## __VA_ARGS__)

double time_elapsed()
{
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);

    double nstart = ((double)START_TIME.tv_sec) + ((double)START_TIME.tv_nsec / 1e9);
    double nend = ((double)end.tv_sec) + ((double)end.tv_nsec / 1e9);
    return nend - nstart;
}

/* End of logger.h */

int main(int argc, char *argv[])
{
    /* Setting the start time for this program */
    clock_gettime(CLOCK_MONOTONIC, &START_TIME);

    int bstart, bend, blen, numth, tid, i;
    int N = 10;
    int a[N], b[N], c[N];
    for (i = 0; i < N; i = i + 1)
    {
        b[i] = i;
        c[i] = i * i;
    }

    log_info("Setting number of threads");
    omp_set_num_threads(3);
    #pragma omp parallel private(bstart, bend, blen, numth, tid, i)
    {
        numth = omp_get_num_threads();
        tid = omp_get_thread_num();

        log_info("Hello World from thread = %d", tid);

        if (tid == 0)
        {
            log_info("Number of threads in parallel region = %d", numth);
        }

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

        log_info("bstart:%d, bend:%d", bstart, bend);
        for (i = bstart; i <= bend; i = i + 1)
        {
            a[i] = b[i] + c[i];
            log_info("a[%d]=%d", i, a[i]);
        }
    }

    log_info("The final array is:");
    for (i = 0; i < N; i = i + 1)
    {
        log_info("a[%d]=%d", i, a[i]);
    }
}
