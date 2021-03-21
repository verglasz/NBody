
#include <math.h>
#include "grav.h"
#include <stdlib.h>


#ifndef RAND_SEED
	#include <time.h>
	#define RAND_SEED time(NULL)
#endif

const double MASS_MIN = 1;
const double MASS_RANGE = 50;
const double REPULSIVE_DISTANCE = 1e-4 * POS_RANGE;

const double G_CONSTANT = 6.6743e-11;

/* get the vector b→a */
Vec3 vec_diff(Vec3 a, Vec3 b) {
	Position d;
	d.x = a.x - b.x;
	d.y = a.y - b.y;
	d.z = a.z - b.z;
	return d;
}

Vec3 vec_add(Vec3 a, Vec3 b) {
	Vec3 res;
	res.x = a.x + b.x;
	res.y = a.y + b.y;
	res.z = a.z + b.z;
	return res;
}

void vec_accumulate(Vec3 * acc, Vec3 val) {
	acc->x += val.x;
	acc->y += val.y;
	acc->z += val.z;
}

void vec_rescale(Vec3 * v, double scale) {
	v->x *= scale;
	v->y *= scale;
	v->z *= scale;
}


double length(Vec3 r) {
	double lsquared = r.x*r.x + r.y*r.y + r.z*r.z;
	return sqrt(lsquared);
}

void init_body(Body *b) {
	b->mass = MASS_MIN + MASS_RANGE / RAND_MAX * random();
	b->vel.x = 1e-2 * (1. - 2. / RAND_MAX * random());
	b->vel.y = 1e-2 * (1. - 2. / RAND_MAX * random());
	b->vel.z = 1e-2 * (1. - 2. / RAND_MAX * random());
	b->pos.x = 0.8 * POS_RANGE / RAND_MAX * random();
	b->pos.y = 0.8 * POS_RANGE / RAND_MAX * random();
	b->pos.z = 0.8 * POS_RANGE / RAND_MAX * random();
}

void init_bodies(Body bodies[], int len) {
	srandom(RAND_SEED);
#pragma omp parallel for
	for (int i=0; i<len; i++) {
		init_body(&bodies[i]);
	}
}

bool body_is_in_bounds(Body *body) {
	Position * pos = &body->pos;
	bool x = pos->x > 0. && pos->x < POS_RANGE;
	bool y = pos->y > 0. && pos->y < POS_RANGE;
	bool z = pos->z > 0. && pos->z < POS_RANGE;
	return x && y && z;
}

Acceleration gravitational_acc(Position at, Position source, double mass) {
	Acceleration acc;
	Vec3 r = vec_diff(source, at); // r points toward source
	double d = length(r);
	// a small repulsive component prevents things from getting too close to each other
	double dmodulus = G_CONSTANT * mass / (d * d * d) * (1 - REPULSIVE_DISTANCE/d);
	acc.x = dmodulus * r.x;
	acc.y = dmodulus * r.y;
	acc.z = dmodulus * r.z;
	return acc;
}

