
#ifndef _QUADRATIC_H
#define _QUADRATIC_H

#include "grav.h"

typedef struct {
	int num_bodies;
	int steps;
#ifdef _OPENMP
	int workers;
#endif
} Args;

void run(int num_bodies, int steps);
int parse_args(Args * parsed, int argc, char **argv);

#endif

