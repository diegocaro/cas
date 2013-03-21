/*
 * use.c
 *
 *  Created on: Mar 20, 2013
 *      Author: diegocaro
 */

#include <stdio.h>
#include <limits.h>

#include "tgs.h"

#define BUFFER 1024*1024*10

uint buffer1[BUFFER];
uint buffer2[BUFFER];

inline uint start(bitRankW32Int *b, uint i) {
	return select1(b, i+1) - i; 	//in the paper this operation is
					//start(i) = select1(b, i) - i + 1
}

inline uint belong(bitRankW32Int *b, uint i) {
	//return rank(b, i);
	return rank(b, select0(b, i)) - 1;
}


void get_neighbors_point(uint *res, struct tgs *g, uint node, uint time) {
	uint startnode, endnode, endnode_log;
	uint i, j;
	startnode = start(g->map, node);
	endnode = start(g->map, node + 1);

	endnode_log = wt_pos_succesor(g->log, g->nodes + time + 1, startnode, endnode);

#ifdef DEBUG
	printf("endnode: %u\nendnodelog: %u\n", endnode, endnode_log);
#endif

	if (endnode_log < endnode) {
		endnode = endnode_log;
	}


#ifdef DEBUG
	printf("nodes: %u\n", g->nodes);
	printf("time (%u): %u\n", time, time+g->nodes);
	printf("search in range [%u, %u)\n", startnode, endnode);
#endif

	*buffer2 = 0;
	wt_range_report(g->log, startnode, endnode, 0, g->nodes ,buffer2);
	//count_symbols_range(g->log, startnode, endnode, buffer2);

#ifdef DEBUG
	printf("wt_range_report: "); print_arraysort(buffer2);
#endif

	j = 0;
	for (i = 1; i < *buffer2; i += 2) {
		//printf("buffer2[%u] = %u\n", i, buffer2[i]);
		if (buffer2[i+1] % 2 == 1) {
			res[++j] = buffer2[i];
		}
	}
	*res = j;
}


void get_reverse_point(uint *res, struct tgs *g, uint node, uint time) {
	uint last_node, curr_node;
	uint i, j;
	uint cant, cpos;
	uint startnode, endnode, endnode_log;

	*buffer2 = 0;

	pos_symbol(g->log, node, buffer2);

#ifdef DEBUG
	printf("pos_symbol: "); print_arraysort(buffer2);
#endif
	last_node = UINT_MAX;

	j = 0;

	for (i = 1; i <= *buffer2; i++) {
		curr_node = belong(g->map, buffer2[i]);

#ifdef DEBUG
		printf("curr_node: %u\n", curr_node);
#endif

		if ( curr_node != last_node ) {
			startnode = start(g->map, curr_node);
			endnode = start(g->map, curr_node + 1);

			endnode_log = wt_pos_succesor(g->log, g->nodes + time + 1, startnode, endnode);
			if (endnode_log < endnode) {
				endnode = endnode_log;
			}
#ifdef DEBUG
			printf("startnode: %u\nendnode: %u\n", startnode, endnode);
#endif
			cant = rank_wt(g->log, node, startnode);
			cpos = rank_wt(g->log, node, endnode);

			if ( (cpos - cant) % 2 == 1 ) {
				res[++j] = curr_node;
			}
		}
		last_node = curr_node;
	}
	*res = j;


}


int main(int argc, char *argv[]) {
	struct tgs index;
	FILE *f;

	uint i, j;

	f = fopen(argv[1], "r");
	tgs_load(&index, f);
	fclose(f);

	for ( i = 0; i < 5; i++) {
		printf("start(%u): %u\n", i, start(index.map, i));
		for( j = 0; j <= index.maxtime; j++) {
			get_neighbors_point(buffer1, &index, i, j);
			printf("t=%u -> ", j);print_arraysort(buffer1);
		}
	}


	for ( i = 0; i < 5; i++) {
		printf("Reverses of node %u\n", i);
		for( j = 0; j <= index.maxtime; j++) {
			get_reverse_point(buffer1, &index, i, j);
			printf("t=%u <- ", j);print_arraysort(buffer1);
		}
	}





	return 0;
}

