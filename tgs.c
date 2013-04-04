/*
 * tgs.c
 *
 *  Created on: Mar 20, 2013
 *      Author: diegocaro
 */

#include <stdio.h>
#include <limits.h>

#include <sys/types.h>

#include "tgs.h"


void tgs_save(struct tgs *a, FILE *f) {
	fwrite(a, sizeof(struct tgs), 1, f);
	save(a->map, f);
	wt_save(a->log, f);
}

void tgs_load(struct tgs *a, FILE *f) {
	fread(a, sizeof(struct tgs), 1, f);

	a->map = malloc(sizeof(bitRankW32Int));
	a->log = malloc(sizeof(struct wt));

	load(a->map, f);
	wt_load(a->log, f);
}

void tgs_free(struct tgs *a) {
	free_wt(a->log);
	destroyBitRankW32Int(a->map);
	free(a->log);
	a->log = NULL;
	a->map = NULL;
}




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
	//return rank(b, i) - 1;
	return rank(b, select0(b, i)) - 1;
}


void get_neighbors_point(uint *res, struct tgs *g, uint node, uint t) {
	uint startnode, endnode, endnode_log;
	uint i, j;
	startnode = start(g->map, node);
	endnode = start(g->map, node + 1);

	//printf("startnode: %u\n", startnode);
	//printf("endnode: %u\n", endnode);

	endnode_log = wt_next_value_pos(g->log, g->nodes + t + 1, startnode, endnode);

	//printf("endnode_log: %u\n", endnode_log);


	if (endnode_log < endnode) {
		endnode = endnode_log;
	}

	*res = 0;
	wt_range_report(g->log, startnode, endnode, 0, g->nodes ,res);
	//printf("res: ");print_arraysort(res);
	/* *res = 0;
	count_symbols_range(g->log, startnode, endnode, res);
	 */

	//printf("res: ");print_arraysort(res);
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
	return get_neighbors_interval(res, g, node, timestart, timeend, 0);
}

void get_neighbors_strong(uint *res, struct tgs *g, uint node, uint timestart, uint timeend) {
	return get_neighbors_interval(res, g, node, timestart, timeend, 1);
}


void get_neighbors_interval(uint *res, struct tgs *g, uint node, uint timestart, uint timeend, uint semantic) {
	uint startnode, endnode;
	uint i, j;
	uint pos_stime, pos_etime;
	uint *buffer;

	buffer = malloc(sizeof(uint)*BUFFER);

	startnode = start(g->map, node);
	endnode = start(g->map, node + 1);

	pos_stime = wt_next_value_pos(g->log, g->nodes + timestart + 1, startnode, endnode);
	if (pos_stime > endnode) {
		pos_stime = endnode;
	}

	pos_etime = wt_next_value_pos(g->log, g->nodes + timeend + 1, startnode, endnode);
	if (pos_etime > endnode) {
		pos_etime = endnode;
	}
	//printf("stime: %u\netime: %u\n", pos_stime, pos_etime);
	*buffer = 0;
	wt_range_report(g->log, pos_stime, pos_etime, 0, g->nodes ,buffer);
	//count_symbols_range(g->log, startnode, endnode, buffer2);

#ifdef DEBUG
	printf("wt_range_report: "); print_arraysort(res);
#endif
	//printf("wt_range_report: "); print_arraysort(buffer2);
	j = 0;
	for (i = 1; i <= *buffer; i += 2) {
		buffer[++j] = buffer[i];
	}
	*buffer = j;

	//printf("filtered: "); print_arraysort(res);

	*res = 0;
	get_neighbors_point(res, g, node, timestart);

	if (semantic == 0) { //semantic weak
		j = *res;
		for (i = 1; i <= *buffer; i++) {
			res[++j] = buffer[i];
		}
		*res = j;

		qsort(&res[1], *res, sizeof(unsigned int), compare);

		remove_duplicates(res);
	}
	else if (semantic == 1) { //semantic strong

		//printf("direct neighbors: "); print_arraysort(buffer3);
		diff_arraysort(res, buffer);
	}
	free(buffer);
}


void get_reverse_point(uint *res, struct tgs *g, uint node, uint time) {
	uint curr_node;
	uint i, j;
	uint cant, cpos;
	uint startnode, endnode, endnode_log;
	uint *buffer;
	uint nextnode;
	buffer = malloc(sizeof(uint)*BUFFER);

	*buffer = 0;
	wt_select_all(g->log, node, buffer);

	j = 0;
	for (i = 1; i <= *buffer; i++) {
		curr_node = belong(g->map, buffer[i]);

		startnode = start(g->map, curr_node);
		nextnode = start(g->map, curr_node + 1);

		endnode_log = wt_next_value_pos(g->log, g->nodes + time + 1, startnode, nextnode);
		endnode = nextnode;
		if (endnode_log < endnode) {
			endnode = endnode_log;
		}

		cant = i;
		while(i <= *buffer && buffer[i] < endnode ) {
			i++;
		}
		cpos = i;

		if ( (cpos - cant) % 2 == 1 ) {
			res[++j] = curr_node;
		}

		while(i <= *buffer && buffer[i] < nextnode) {
			i++;
		}
		i--;

	}
	*res = j;

	free(buffer);
}



void get_reverse_point_slow(uint *res, struct tgs *g, uint node, uint time) {
	uint last_node, curr_node;
	uint i, j;
	uint cant, cpos;
	uint startnode, endnode, endnode_log;
	uint *buffer;

	buffer = malloc(sizeof(uint)*BUFFER);

	*buffer = 0;

	wt_select_all(g->log, node, buffer);

	last_node = UINT_MAX;

	j = 0;

	for (i = 1; i <= *buffer; i++) {
		curr_node = belong(g->map, buffer[i]);

#ifdef DEBUG
		printf("curr_node: %u\n", curr_node);
#endif

		if ( curr_node != last_node ) {
			startnode = start(g->map, curr_node);
			endnode = start(g->map, curr_node + 1);

			endnode_log = wt_next_value_pos(g->log, g->nodes + time + 1, startnode, endnode);
			if (endnode_log < endnode) {
				endnode = endnode_log;
			}
#ifdef DEBUG
			printf("startnode: %u\nendnode: %u\n", startnode, endnode);
#endif
			cant = rank_wt(g->log, node, startnode);
			cpos = rank_wt(g->log, node, endnode);

			//printf("cant: %u\ncpos: %u\n", cant,cpos);

			if ( (cpos - cant) % 2 == 1 ) {
				res[++j] = curr_node;
			}
		}
		last_node = curr_node;
	}
	*res = j;

	free(buffer);
}

void get_reverse_weak(uint *res, struct tgs *g, uint node, uint ts, uint te) {
	return get_reverse_interval(res, g, node, ts, te, 0);
}

void get_reverse_strong(uint *res, struct tgs *g, uint node, uint ts, uint te) {
	return get_reverse_interval(res, g, node, ts, te, 1);
}

void get_reverse_interval(uint *res, struct tgs *g, uint node, uint ts, uint te, uint semantic) {
	uint last_node, curr_node;
	uint i, j;
	uint cstart, cts, cte;
	uint startnode, endnode, pos_stime, pos_etime;
	uint *buffer;
	uint nextnode;

	buffer = malloc(sizeof(uint)*BUFFER);

	*buffer = 0;

	wt_select_all(g->log, node, buffer);

	last_node = UINT_MAX;

	j = 0;

	for (i = 1; i <= *buffer; i++) {
		curr_node = belong(g->map, buffer[i]);

		startnode = start(g->map, curr_node);
		nextnode = start(g->map, curr_node + 1);

		endnode = nextnode;

		pos_stime = wt_next_value_pos(g->log, g->nodes + ts + 1, startnode, endnode);
		if (pos_stime > endnode) {
			pos_stime = endnode;
		}

		pos_etime = wt_next_value_pos(g->log, g->nodes + te + 1, startnode, endnode);
		if (pos_etime > endnode) {
			pos_etime = endnode;
		}



#ifdef DEBUG
		printf("startnode: %u\nendnode: %u\n", startnode, endnode);
#endif
		//cstart = rank_wt(g->log, node, startnode);
		cstart = i-1;


		//cts = rank_wt(g->log, node, pos_stime);
		while(i <= *buffer && buffer[i] < pos_stime ) {
					i++;
		}
		cts = i-1;

		//cte = rank_wt(g->log, node, pos_etime);
		while(i <= *buffer && buffer[i] < pos_etime ) {
			i++;
		}
		cte = --i;


		if (semantic == 0) {
			// semantic = 0 weak
			if ( (cts - cstart) % 2 == 1 || cte > cts) {
				res[++j] = curr_node;
			}
		}
		else if (semantic == 1) {
			//semantic = 1 strong
			if ( (cts - cstart) % 2 == 1 && cte == cts) {
				res[++j] = curr_node;
			}
		}

		while(i <= *buffer && buffer[i] < nextnode) {
			i++;
		}
		i--;


	}
	*res = j;

	free(buffer);
}

void get_reverse_interval_slow(uint *res, struct tgs *g, uint node, uint ts, uint te, uint semantic) {
	uint last_node, curr_node;
	uint i, j;
	uint cstart, cts, cte;
	uint startnode, endnode, pos_stime, pos_etime;
	uint *buffer;

	buffer = malloc(sizeof(uint)*BUFFER);

	*buffer = 0;

	wt_select_all(g->log, node, buffer);

	last_node = UINT_MAX;

	j = 0;

	for (i = 1; i <= *buffer; i++) {
		curr_node = belong(g->map, buffer[i]);

#ifdef DEBUG
		printf("curr_node: %u\n", curr_node);
#endif

		if ( curr_node != last_node ) {
			startnode = start(g->map, curr_node);
			endnode = start(g->map, curr_node + 1);

			pos_stime = wt_next_value_pos(g->log, g->nodes + ts + 1, startnode, endnode);
			if (pos_stime > endnode) {
				pos_stime = endnode;
			}

			pos_etime = wt_next_value_pos(g->log, g->nodes + te + 1, startnode, endnode);
			if (pos_etime > endnode) {
				pos_etime = endnode;
			}



#ifdef DEBUG
			printf("startnode: %u\nendnode: %u\n", startnode, endnode);
#endif
			cstart = rank_wt(g->log, node, startnode);
			cts = rank_wt(g->log, node, pos_stime);
			cte = rank_wt(g->log, node, pos_etime);

			if (semantic == 0) {
				// semantic = 0 weak
				if ( (cts - cstart) % 2 == 1 || cte > cts) {
					res[++j] = curr_node;
				}
			}
			else {
				//semantic = 1 strong
				if ( (cts - cstart) % 2 == 1 && cte == cts) {
					res[++j] = curr_node;
				}
			}

		}
		last_node = curr_node;
	}
	*res = j;

	free(buffer);
}

