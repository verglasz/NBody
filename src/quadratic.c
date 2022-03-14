
#include "quadratic.h"
#include "common.h"
#include "grav.h"
#include <stdlib.h>
#include <stdio.h>


#ifdef _OPENMP
# include <omp.h>
# define NUM_ARGS 3
# define WORKER_ARG " <workers>"
#else
# define omp_set_num_threads(...)
# define NUM_ARGS 2
# define WORKER_ARG
#endif

const char ARGHELP[] = "<bodies> <steps>" WORKER_ARG;

int main(int argc, char ** argv) {
	Args args;
	if (parse_args(&args, argc-1, argv+1)) {
		eprintf("Usage: %s %s\n", argv[0], ARGHELP);
		return 1;
	}
	omp_set_num_threads(args.workers);
    run(args.num_bodies, args.steps);
	return 0;
}

int parse_args(Args * parsed, int argc, char **argv) {
	if (argc != NUM_ARGS) {
		return 1;
	}
	parsed->num_bodies = atoi(argv[0]);
	if (parsed->num_bodies <= 0) {
		return 1;
	}
	parsed->steps = atoi(argv[1]);
	if (parsed->steps <= 0) {
		return 1;
	}
#ifdef _OPENMP
	parsed->workers = atoi(argv[2]);
	if (parsed->workers <= 0) {
		return 1;
	}
#endif
	return 0;
}

Acceleration get_total_accel(int current, Body bodies[], int num_bodies) {
	Acceleration total = {0, 0, 0};
	for (int i=0; i<num_bodies; i++) {
		if (i != current && body_is_in_bounds(&bodies[i])) {
			Acceleration a = gravitational_acc(
				bodies[current].pos,
				bodies[i].pos,
				bodies[i].mass
			);
			vec_accumulate(&total, a);
		}
	}
	return total;
}

void simulation_step(Body bodies[], int num_bodies, Acceleration accels[]) {
#pragma omp parallel for
	for (int i=0; i<num_bodies; i++) {
		accels[i] = get_total_accel(i, bodies, num_bodies);
	}
#pragma omp parallel for
	for (int i=0; i<num_bodies; i++) {
		Body * b = &bodies[i];
		vec_accumulate(&b->pos, b->vel);
		vec_accumulate(&b->vel, accels[i]);
	}
}

void run(int num_bodies, int steps) {
	Body * bodies = (Body *) malloc(sizeof(Body) * num_bodies);
	Acceleration * accels = (Acceleration *) malloc(sizeof(Acceleration) * num_bodies);
	init_bodies(bodies, num_bodies);
    struct timespec start;
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &start);
	for (int i=0; i<steps; i++) {
		simulation_step(bodies, num_bodies, accels);
	}
    clock_gettime(CLOCK_MONOTONIC, &end);
    show_elapsed_time(start, end);
	free(bodies);
	free(accels);
}

