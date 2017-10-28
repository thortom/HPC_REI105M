#include "logger.h"

void init_logger()
{
    /* Setting the start time for the logger */
    START_TIME = get_current_time();
}

double get_current_time()
{
    return MPI_Wtime();
}
