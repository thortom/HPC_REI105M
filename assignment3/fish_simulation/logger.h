#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <unistd.h>

/* Global start time */
double START_TIME;

void init_logger();
double get_current_time();

/* log_info macro */
#define log_info(format, ...) printf("%f sec [%s:%d] " format "\n", (get_current_time() - START_TIME), __FILE__, __LINE__, ## __VA_ARGS__)
#define log_debug(format, ...) log_info("DEBUG\n" format "")

#endif