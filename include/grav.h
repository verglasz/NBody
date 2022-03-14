
#ifndef _GRAV_H
#define _GRAV_H

#include <stdbool.h>

#define POS_RANGE 1.0

typedef struct {
	double x;
	double y;
	double z;
} Vec3;

typedef Vec3 Position;
typedef Vec3 Velocity;
typedef Vec3 Acceleration;

typedef struct {
	double mass;
	Position pos;
	Velocity vel;
} Body;


bool body_is_in_bounds(Body * body);
double length(Vec3 r);
Vec3 vec_add(Vec3 a, Vec3 b);
Vec3 vec_diff(Vec3 a, Vec3 b);
void vec_accumulate(Vec3 * acc, Vec3 val);
void vec_rescale(Vec3 *v, double scale);
void vec_accumulate_scaled(Vec3 * acc, Vec3 val, double scale);
void init_bodies(Body bodies[], int len);
Acceleration gravitational_acc(Position at, Position source, double source_mass);


#endif

