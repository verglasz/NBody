
#ifndef _COMMON_H
#define _COMMON_H

#include <time.h>

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

#ifdef DEBUG
#define debug eprintf
#else
#define debug(...)
#endif


void show_elapsed_time(struct timespec start, struct timespec end);

#endif

