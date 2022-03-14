
#include <math.h>
#include "grav.h"
#include <stdlib.h>


#ifndef RAND_SEED
	#include <time.h>
	#define RAND_SEED time(NULL)
#endif

const double MASS_MIN = 1;
const double MASS_RANGE = 50;
const double REPULSIVE_DISTANCE = 1e-10 * POS_RANGE;

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

void vec_accumulate_scaled(Vec3 * acc, Vec3 val, double scale) {
	acc->x += val.x * scale;
	acc->y += val.y * scale;
	acc->z += val.z * scale;
}


double length(Vec3 r) {
	double lsquared = r.x*r.x + r.y*r.y + r.z*r.z;
	return sqrt(lsquared);
}

double rand_between(double min, double max) {
	double r = random() * 1.0 / RAND_MAX;
	return min + r * (max - min);
}

void init_body(Body *b) {
	b->mass = rand_between(MASS_MIN, MASS_MIN + MASS_RANGE);
	b->vel.x = 3e-3 * rand_between(-1, 1);
	b->vel.y = 3e-3 * rand_between(-1, 1);
	b->vel.z = 3e-3 * rand_between(-1, 1);
	b->pos.x = 0.8 * rand_between(-POS_RANGE, +POS_RANGE);
	b->pos.y = 0.8 * rand_between(-POS_RANGE, +POS_RANGE);
	b->pos.z = 0.8 * rand_between(-POS_RANGE, +POS_RANGE);
}


void init_bodies(Body bodies[], int len) {
	srandom(RAND_SEED);
	for (int i=0; i<len; i++) {
		init_body(&bodies[i]);
	}
}

bool body_is_in_bounds(Body *body) {
	Position * pos = &body->pos;
	bool x = pos->x > -POS_RANGE && pos->x < POS_RANGE;
	bool y = pos->y > -POS_RANGE && pos->y < POS_RANGE;
	bool z = pos->z > -POS_RANGE && pos->z < POS_RANGE;
	return x && y && z;
}

Acceleration gravitational_acc(Position at, Position source, double source_mass) {
	Vec3 r = vec_diff(source, at); // r points toward source
	double d = length(r);
	// a small repulsive component prevents things from getting too close to each other
	double dmodulus = G_CONSTANT * source_mass / (d * d * d) * (1 - REPULSIVE_DISTANCE/d);
	Acceleration acc;
	acc.x = dmodulus * r.x;
	acc.y = dmodulus * r.y;
	acc.z = dmodulus * r.z;
	return acc;
}

