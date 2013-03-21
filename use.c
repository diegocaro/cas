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
uint buffer3[BUFFER];
/**********************************************************/
//Extra OPs

int compare (const void * a, const void * b) {
  return ( *(unsigned int*)a - *(unsigned int*)b );
}

// b[] must be sorted
inline void remove_duplicates(unsigned int *b) {
  int i, j;


  //doing the no duplicates operation.. for example 2 2 2 3 4 5 6 6 7 -> 2 3 4 5 6 7
  i = 0;
  for(j = 2; j<=*b; j++) {
    if ( b[j-1] == b[j]) continue; // se salta el numero
    else {
      b[++i] = b[j-1];
    }
  }
  //printf("b[%d] = %d\n", i,b[i]);
  if (j == *b + 1) {
    b[++i] = b[*b];
  }
  *b = i;

}


void merge_arraysort( unsigned int *res,  const unsigned int *a, const unsigned int *b) {
  int i, j, u;

  i = j = u = 1;

  while (1) {

    if ( i == a[0]+1 && j == b[0]+1 ) {
      break;
    }

    if (i == a[0]+1) {
      res[u] = b[j];
      j++; u++;
      continue;
    }

    if (j == b[0]+1) {
      res[u] = a[i];
      u++; i++;
      continue;
    }

    if ( a[i] < b[j] ) {
      res[u] = a[i];
      i++; u++;
      continue;
    }

    if ( a[i] > b[j] ) {
      res[u] = b[j];
      j++; u++;
      continue;
    }

    if ( a[i] == b[j] ) {
      res[u] = a[i];
      i++; j++; u++;
      continue;
    }

  }

  *res = u-1;
}



void diff_arraysort( unsigned int *a, const unsigned int *b) {
  int i, j, u;
  i = j = u = 1;
  while(1) {
    if (i == a[0]+1) {
      break;
    }

    if (j == b[0]+1) {
      a[u] = a[i];
      u++; i++;
      continue;
    }


    if (a[i] == b[j]) {
      i++; j++;
      continue;
    }

    if (a[i] < b[j]) {
      a[u] = a[i];
      u++; i++;
      continue;
    }

    if (a[i] > b[j]) {
      j++;
      continue;
    }


  }

  *a = u-1;
}



/***************************************************************/

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

	*res = 0;
	wt_range_report(g->log, startnode, endnode, 0, g->nodes ,res);
	//count_symbols_range(g->log, startnode, endnode, buffer2);

#ifdef DEBUG
	printf("wt_range_report: "); print_arraysort(res);
#endif

	j = 0;
	for (i = 1; i < *res; i += 2) {
		//printf("buffer2[%u] = %u\n", i, buffer2[i]);
		if (res[i+1] % 2 == 1) {
			res[++j] = res[i];
		}
	}
	*res = j;
}


void get_neighbors_weak(uint *res, struct tgs *g, uint node, uint timestart, uint timeend) {
	uint startnode, endnode, endnode_log;
	uint i, j;
	uint pos_stime, pos_etime;


	startnode = start(g->map, node);
	endnode = start(g->map, node + 1);

	pos_stime = wt_pos_succesor(g->log, g->nodes + timestart + 1, startnode, endnode);
	if (pos_stime > endnode) {
		pos_stime = endnode;
	}

	pos_etime = wt_pos_succesor(g->log, g->nodes + timeend + 1, startnode, endnode);
	if (pos_etime > endnode) {
		pos_etime = endnode;
	}
	//printf("stime: %u\netime: %u\n", pos_stime, pos_etime);
	*res = 0;
	wt_range_report(g->log, pos_stime, pos_etime, 0, g->nodes ,res);
	//count_symbols_range(g->log, startnode, endnode, buffer2);

#ifdef DEBUG
	printf("wt_range_report: "); print_arraysort(res);
#endif
	//printf("wt_range_report: "); print_arraysort(buffer2);
	j = 0;
	for (i = 1; i <= *res; i += 2) {
		res[++j] = res[i];
	}
	*res = j;

	//printf("filtered: "); print_arraysort(res);
	*buffer2 = 0;
	get_neighbors_point(buffer2, g, node, timestart);
	//printf("direct neighbors: "); print_arraysort(buffer3);

	j = *res;
	for (i = 1; i <= *buffer2; i++) {
		res[++j] = buffer2[i];
	}
	*res = j;

	qsort(&res[1], *res, sizeof(unsigned int), compare);

	remove_duplicates(res);
}


void get_neighbors_strong(uint *res, struct tgs *g, uint node, uint timestart, uint timeend) {
	uint startnode, endnode, endnode_log;
	uint i, j;
	uint pos_stime, pos_etime;


	startnode = start(g->map, node);
	endnode = start(g->map, node + 1);

	pos_stime = wt_pos_succesor(g->log, g->nodes + timestart + 1, startnode, endnode);
	if (pos_stime > endnode) {
		pos_stime = endnode;
	}

	pos_etime = wt_pos_succesor(g->log, g->nodes + timeend + 1, startnode, endnode);
	if (pos_etime > endnode) {
		pos_etime = endnode;
	}
	//printf("stime: %u\netime: %u\n", pos_stime, pos_etime);
	*buffer2 = 0;
	wt_range_report(g->log, pos_stime, pos_etime, 0, g->nodes ,buffer2);
	//count_symbols_range(g->log, startnode, endnode, buffer2);

#ifdef DEBUG
	printf("wt_range_report: "); print_arraysort(res);
#endif
	//printf("wt_range_report: "); print_arraysort(buffer2);
	j = 0;
	for (i = 1; i <= *buffer2; i += 2) {
		buffer2[++j] = buffer2[i];
	}
	*buffer2 = j;

	//printf("filtered: "); print_arraysort(res);

	*res = 0;
	get_neighbors_point(res, g, node, timestart);
	//printf("direct neighbors: "); print_arraysort(buffer3);

	diff_arraysort(res, buffer2);
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

	for ( i = 0; i < 5; i++) {
		printf("direct weak %u\n", i);
		for( j = 0; j <= index.maxtime; j++) {
			printf("time interval [0, %u)\n", j);
			get_neighbors_weak(buffer1, &index, i, 0, j);
			printf("t=%u -> ", j);print_arraysort(buffer1);
		}
	}

	for ( i = 0; i < 5; i++) {
		printf("direct strong %u\n", i);
		for( j = 0; j <= index.maxtime; j++) {
			printf("time interval [0, %u)\n", j);
			get_neighbors_strong(buffer1, &index, i, 0, j);
			printf("t=%u -> ", j);print_arraysort(buffer1);
		}
	}


	return 0;
}

