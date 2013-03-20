#ifndef ADJLOGINCLUDED
#define ADJLOGINCLUDED

#include <sys/types.h>

struct adjlog {
	uint nodes;
	uint changes;
	uint maxtime;

	uint *map; //position of nodes
	uint size_map;

	uint size_log; //size of the log
	uint *log; //including time and edges
};

#endif
