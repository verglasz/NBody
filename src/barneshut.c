

#include "barneshut.h"
#include "common.h"
#include "grav.h"
#include <stdlib.h>
#include <stdio.h>

#ifdef _OPENMP
	#include <omp.h>
	#define NUM_ARGS 4
	#define WORKER_ARG " <workers>"
#else
	#define NUM_ARGS 3
	#define WORKER_ARG
#endif

const char ARGHELP[] = "<bodies> <steps> <BH distance threshold>" WORKER_ARG;

#ifdef _OPENMP
omp_lock_t tree_locks[8];

void init_openmp(int num_threads) {
	omp_set_num_threads(num_threads);
	for (int i=0; i<8; i++) {
		omp_init_lock(&tree_locks[i]);
	}
}
#else
#define init_openmp(...)
#define omp_set_lock(...)
#define omp_unset_lock(...)
#endif


int main(int argc, char ** argv) {
	Args args;
	if (parse_args(&args, argc-1, argv+1)) {
		eprintf("Usage: %s %s\n", argv[0], ARGHELP);
		return 1;
	}
	init_openmp(args.workers);
    struct timespec start;
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    run(args.num_bodies, args.steps, args.far);
    clock_gettime(CLOCK_MONOTONIC, &end);
    show_elapsed_time(start, end);
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
	parsed->far = strtod(argv[2], NULL);
	if (parsed->far <= 0.) {
		return 1;
	} else if (parsed->far > 1.) {
		eprintf("Warning: using a far threshold greater than 1 is unlikely to give good results\n");
	}
#ifdef _OPENMP
	parsed->workers = atoi(argv[3]);
	if (parsed->workers <= 0) {
		return 1;
	}
#endif
	return 0;
}


void run(int num_bodies, int steps, double far_threshold) {
	Body * bodies = (Body *) malloc(sizeof(Body) * num_bodies);
	Acceleration * accels = (Acceleration *) malloc(sizeof(Acceleration) * num_bodies);
	BHTree * tree = new_tree(num_bodies);
	init_bodies(bodies, num_bodies);
	for (int i=0; i<steps; i++) {
		simulation_step(tree, bodies, num_bodies, accels, far_threshold);
		debug("\n---- step completed ----\n\n");
	}
	destroy_tree(tree);
	free(bodies);
	free(accels);
}


BHTree * new_tree(int num_bodies) {
	BHTree * tree = (BHTree *) malloc(sizeof(BHTree));
	tree->halfsize = POS_RANGE/2;
	Position center = {POS_RANGE/2, POS_RANGE/2, POS_RANGE/2};
	tree->center = center;
	int initial_bufsize = (num_bodies / 8) + 8;
	for (int i=0; i<8; i++) {
		NodeBuffer * buf = &tree->octants[i];
		buf->capacity = initial_bufsize;
		buf->next_free = 0;
		buf->nodes = (BHNode *) malloc(sizeof(BHNode) * initial_bufsize);
	}
	return tree;
}


void destroy_tree(BHTree * tree) {
	for (int i=0; i<8; i++) {
		free(tree->octants[i].nodes);
	}
	free(tree);
}


void simulation_step(BHTree * tree, Body bodies[], int num_bodies, Acceleration accels[], double far_threshold) {
	fill_tree(tree, bodies, num_bodies);
#pragma omp parallel for
	for (int i=0; i<num_bodies; i++) {
		accels[i] = get_total_accel(&bodies[i], tree, far_threshold);
	}
#pragma omp parallel for
	for (int i=0; i<num_bodies; i++) {
		evolve_body(&bodies[i], &accels[i]);
	}
}

void evolve_body(Body * body, const Acceleration * acc) {
	vec_accumulate(&body->pos, body->vel);
	vec_accumulate(&body->vel, *acc);
}



void fill_tree(BHTree * tree, Body bodies[], int num_bodies) {
	zero_tree(tree);
	for (int i=0; i<num_bodies; i++) {
		Body * b = &bodies[i];
		if (body_is_in_bounds(b)) {
			root_insert_body(tree, b);
		}
	}
	rescale_tree(tree);
}

void zero_tree(BHTree * tree) {
	for (int i=0; i<8; i++) {
		NodeBuffer * octant = &tree->octants[i];
		init_node_relative(&octant->nodes[0], &tree->center, tree->halfsize, i);
		octant->next_free = 1;
	}
}


/* initialize a new node from its octree index relative to its parent */
void init_node_relative(BHNode * node, const Position * parent_pos, double parent_halfsize, int idx) {
	double span = parent_halfsize * 0.5;
	Position mid = *parent_pos;
	if (idx & 1) {
		mid.x += span;
	} else {
		mid.x -= span;
	}
	if (idx & 2) {
		mid.y += span;
	} else {
		mid.y -= span;
	}
	if (idx & 4) {
		mid.z += span;
	} else {
		mid.z -= span;
	}
	init_node(node, &mid, span);
}


/* initialize a new empty node with the given position and size */
void init_node(BHNode * node, const Position * midpoint, double halfsize) {
	node->midpoint = *midpoint;
	node->halfsize = halfsize;
	node->mass = 0.;
	node->center_of_mass.x = 0.;
	node->center_of_mass.y = 0.;
	node->center_of_mass.z = 0.;
	for (int i=0; i<8; i++) {
		node->children[i].type = EMPTY;
	}
}


/* calculate in which octant `pos` lies with respect to `midpoint` */
int octree_idx(const Position * midpoint, const Position * pos) {
	int x = pos->x > midpoint->x;
	int y = pos->y > midpoint->y;
	int z = pos->z > midpoint->z;
	return x*1 | y*2 | z*4;
}



/* may reallocate the backing store through the call to `insert_body`
 */
void root_insert_body(BHTree *tree, const Body * body) {
	int idx = octree_idx(&tree->center, &body->pos);
	NodeBuffer *octant = &tree->octants[idx];
#pragma omp task
	{
		omp_set_lock(&tree_locks[idx]);
		insert_body(0, body, octant);
		omp_unset_lock(&tree_locks[idx]);
	}
}

/* may reallocate the backing store through the call to `insert_body_in_child`
 */
void insert_body(int current_node, const Body * body, NodeBuffer * buffer) {
	BHNode * current = &buffer->nodes[current_node];
	update_node_with_body(current, body);
	int idx = octree_idx(&current->midpoint, &body->pos);
	insert_body_in_child(current_node, idx, body, buffer);
}


/* may reallocate the backing store through the call to `push_down_child`
 */
void insert_body_in_child(int parent_node, int idx, const Body *body, NodeBuffer * buffer) {
	BHChild * child = &buffer->nodes[parent_node].children[idx];
	int next;
	switch (child->type) {
	case EMPTY:
		child->type = BODY;
		child->body = body;
		break;
	case BODY:
		// may move the backing store in the buffer,
		// so the child pointer is invalid after this
		// (and node_idx can't be cached since
		// it's going to be changed by push_down_child)
		push_down_child(parent_node, idx, buffer);
		// FALLTHROUGH
	case NODE:
		// getting next from all-fresh data
		next = buffer->nodes[parent_node].children[idx].node_idx;
		insert_body(next, body, buffer);
		break;
	}
}

void update_node_with_body(BHNode *node, const Body * body) {
	node->mass += body->mass;
	Position addcm = body->pos;
	vec_rescale(&addcm, body->mass);
	vec_accumulate(&node->center_of_mass, addcm);
}

/* may reallocate the backing store through the call to `get_next_free`
 */
void push_down_child(int parent_node, int idx, NodeBuffer * buffer) {
	int new_node = get_next_free(buffer);
	BHNode * new = &buffer->nodes[new_node];
	BHNode * parent = &buffer->nodes[parent_node];
	init_node_relative(new, &parent->midpoint, parent->halfsize, idx);
	BHChild * child = &parent->children[idx];
	insert_body(new_node, child->body, buffer);
	// ^ this call won't invalidate the child pointer
	// because it's going to be inserting in an EMPTY child
	child->type = NODE;
	child->node_idx = new_node;
}

/* Get the index of a fresh node from the buffer, reallocating the backing store if we've run out.
 * NOTE: this means pointers to nodes or their parts may become invalid after this call
 */
int get_next_free(NodeBuffer * buffer) {
	debug("requested a node from buffer %p\n", buffer);
	if (buffer->next_free >= buffer->capacity) {
		// reallocate backing buffer to increase capacity
		debug("Reallocating buffer, currently with size %d, since next free is %d\n", buffer->capacity, buffer->next_free);
		buffer->capacity *= 2;
		if (buffer->capacity > 1 << 24) { // no way we're really using 8 million nodes per buffer
			eprintf("Tried to request a ridiculously large buffer, exiting...\n");
			exit(1);
		}
		buffer->nodes = (BHNode *) realloc(buffer->nodes, sizeof(BHNode) * buffer->capacity);
	}
	return buffer->next_free++;
}


/* Since the tree is build with mass*cm in place of the center of mass,
 * we rescale it at the end, dividing each node's cm value by its mass
 */
void rescale_tree(BHTree * tree) {
#pragma omp parallel for schedule(dynamic, 1)
	for (int i=0; i<8; i++) {
		NodeBuffer * octant = &tree->octants[i];
		rescale_cm(&octant->nodes[0], octant);
	}

}

void rescale_cm(BHNode * node, const NodeBuffer * buffer) {
	vec_rescale(&node->center_of_mass, 1 / node->mass);
	for (int i=0; i<8; i++) {
		BHChild * child = &node->children[i];
		if (child->type == NODE) {
			BHNode * next = &buffer->nodes[child->node_idx];
			rescale_cm(next, buffer);
		}
	}
}


Acceleration get_total_accel(const Body * on, const BHTree * tree, double far_threshold) {
	Acceleration total = {0, 0, 0};
	Acceleration a;
	for (int i=0; i<8; i++) {
		const NodeBuffer *octant = &tree->octants[i];
		a = get_accel_from_node(on, &octant->nodes[0], octant, far_threshold);
		vec_accumulate(&total, a);
	}
	return total;
}

Acceleration get_accel_from_node(const Body * on, const BHNode * node, const NodeBuffer * buffer, double far_threshold) {
	double distance = length(vec_diff(node->center_of_mass, on->pos));
	double width = 2 * node->halfsize;
	bool is_far = width/distance < far_threshold;
	if (is_far) {
		return gravitational_acc(on->pos, node->center_of_mass, node->mass);
	} else {
		Acceleration total = {0, 0, 0};
		Acceleration a;
		for (int i=0; i<8; i++) {
			const BHChild * child = &node->children[i];
			a = get_accel_from_child(on, child, buffer, far_threshold);
			vec_accumulate(&total, a);
		}
		return total;
	}
}

Acceleration get_accel_from_child(const Body *on, const BHChild *child, const NodeBuffer * buffer, double far_threshold) {
	Acceleration a = {0., 0., 0.};
	if (child->type == NODE) {
		const BHNode *next = &buffer->nodes[child->node_idx];
		a = get_accel_from_node(on, next, buffer, far_threshold);
	} else if (child->type == BODY && child->body != on) {
		a = gravitational_acc(on->pos, child->body->pos, child->body->mass);
	}
	return a;
}

