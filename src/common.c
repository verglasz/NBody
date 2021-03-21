
#include "common.h"
#include <stdio.h>

void show_elapsed_time(struct timespec start, struct timespec end) {
    int nsec = end.tv_nsec - start.tv_nsec;
    int seconds = end.tv_sec - start.tv_sec;
    if (nsec < 0) {
        nsec += 1e9;
        seconds -= 1;
    }
    int mins = seconds / 60;
    double secs = (seconds % 60) + nsec * 1e-9;
    printf(" Total elapsed time: ");
    if (mins > 0) {
        fprintf(stderr, "%d minutes, ", mins);
    }
    printf("%.3f seconds\n", secs);
}

