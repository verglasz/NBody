
#ifndef _QUADRATIC_H
#define _QUADRATIC_H

#include "grav.h"
#include "stdbool.h"

typedef struct {
	int num_bodies;
	int steps;
	double far;
#ifdef _OPENMP
	int workers;
#endif
} Args;

typedef enum childtype_enum {EMPTY, BODY, NODE} ChildType;

typedef struct bhnode_child {
	enum childtype_enum type;
	union {
		const Body * body;
		int node_idx;
	};
} BHChild;

typedef struct bhnode_struct {
	Position midpoint;
	double halfsize;
	double mass;
	Position center_of_mass;
	BHChild children[8];
} BHNode;

typedef struct nodebuffer_struct {
	BHNode * nodes;
	int capacity;
	int next_free;
} NodeBuffer;

typedef struct bhtree_struct {
	Position center;
	double halfsize;
	NodeBuffer octants[8];
} BHTree;

BHTree * new_tree(int num_bodies);
void zero_tree(BHTree *tree);
void fill_tree(BHTree *tree, Body bodies[], int num_bodies);
void rescale_tree(BHTree *tree);
void destroy_tree(BHTree *tree);

int get_next_free(NodeBuffer * buffer);

void run(int num_bodies, int steps, double far);
void simulation_step(BHTree *tree, Body bodies[], int num_bodies, Acceleration accels[], double far_threshold);
void evolve_body(Body * body, const Acceleration * acc, double timedelta);


void rescale_cm(BHNode * node, const NodeBuffer * buffer);
void root_insert_body(BHTree *tree, const Body *body);
void insert_body(int current_node, const Body *body, NodeBuffer *buffer);
void insert_body_in_child(int parent_node, int idx, const Body *body, NodeBuffer * buffer);
void push_down_child(int parent_node, int idx, NodeBuffer * buffer);
void update_node_with_body(BHNode *node, const Body * body);
void init_node(BHNode *node, const Position *midpoint, double halfsize);
void init_node_relative(BHNode *node, const Position *parent_pos, double parent_halfsize, int idx);

int octree_idx(const Position *midpoint, const Position *pos);

Acceleration get_total_accel(const Body *on, const BHTree *tree, double far_threshold);
Acceleration get_accel_from_node(const Body *on, const BHNode *node, const NodeBuffer *buffer, double far_threshold);
Acceleration get_accel_from_child(const Body *on, const BHChild *child, const NodeBuffer *buffer, double far_threshold);

int parse_args(Args *parsed, int argc, char **argv);

#endif

