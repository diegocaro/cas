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
#include "arraysort.h"

using namespace std;
using namespace cds_static;

void tgs_save(struct tgs *a, ofstream & f) {
	f.write(reinterpret_cast<char *>(a), sizeof(struct tgs));
  a->map->save(f);
  a->log->save(f);
	//save(a->map, f);
	//wt_save(a->log, f);
}

void tgs_load(struct tgs *a, ifstream & f) {
//fread(a, sizeof(struct tgs), 1, f);
  f.read(reinterpret_cast<char *>(a), sizeof(struct tgs));

  a->map = BitSequence::load(f);
  a->log = WaveletMatrix::load(f);

	//a->map = malloc(sizeof(bitRankW32Int));
	//a->log = malloc(sizeof(struct wt));

	//load(a->map, f);
	//wt_load(a->log, f);
}

void tgs_free(struct tgs *a) {
	//free_wt(a->log);
	//destroyBitRankW32Int(a->map);
	//free(a->log);
  
  delete a->log;
  delete a->map;
  
	a->log = NULL;
	a->map = NULL;
}


size_t tgs_size(struct tgs *a) {
//	printf("%lu + %lu \n", a->log->getSize(), a->map->getSize());
	
	//return (size_t)sizeof(struct tgs) + (size_t)a->log->getSize() + (size_t)a->map->getSize();
	return 0;
}


 uint start(BitSequence *b, uint i) {
//	return b->select1(i+1) - i; 	//in the paper this operation is
                                  //start(i) = select1(b, i) - i + 1

  uint ret = b->select1(i+1);
  if (ret == (uint)(-1)) {
    return b->getLength() - i;
  }
  else {
    return ret -i;
  }
}

inline uint belong(BitSequence *b, uint i) {
	//return rank(b, i) - 1;
	return b->rank1( b->select0(i) ) - 1;
}

uint get_snapshot(struct tgs *g, uint t) {
	uint startnode, endnode, endnode_log;
	
	vector<uint> myres;
	
	uint count = 0;
	uint i;
	uint node;
	for (node = 0; node < g->nodes; node++) {
		startnode = start(g->map, node);
		endnode = start(g->map, node + 1);
		
		endnode_log = g->log->next_value_pos( g->nodes + t + 1, startnode, endnode);

	//	printf("endnode_log: %u\n", endnode_log);
		if (endnode_log < endnode) {
			endnode = endnode_log;
		}
		
		myres.clear();
		g->log->range_report(startnode, endnode, 0, g->nodes , myres);
		for (i = 0; i < myres.size(); i += 2) {
			//printf("buffer2[%u] = %u\n", i, buffer2[i]);
			if (myres[i+1] % 2 == 1) {
				count++;
			}
		}
	}
	
	return count;
}

void get_neighbors_point(uint *res, struct tgs *g, uint node, uint t) {
	uint startnode, endnode, endnode_log;
	uint i, j;
  //printf("node: %u\n", node);
	startnode = start(g->map, node);
	endnode = start(g->map, node + 1);

//	printf("startnode: %u\n", startnode);
//	printf("endnode: %u\n", endnode);

	endnode_log = g->log->next_value_pos( g->nodes + t + 1, startnode, endnode);

//	printf("endnode_log: %u\n", endnode_log);


	if (endnode_log < endnode) {
		endnode = endnode_log;
	}

	*res = 0;
  vector<uint> myres;
	g->log->range_report(startnode, endnode, 0, g->nodes , myres);
	//printf("res: ");print_arraysort(res);
	/* *res = 0;
	count_symbols_range(g->log, startnode, endnode, res);
	 */

	//printf("res: ");print_arraysort(res);
	j = 0;
	for (i = 0; i < myres.size(); i += 2) {
		//printf("buffer2[%u] = %u\n", i, buffer2[i]);
		if (myres[i+1] % 2 == 1) {
			res[++j] = myres[i];
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

	buffer = (uint *)malloc(sizeof(uint)*BUFFER);

	startnode = start(g->map, node);
	endnode = start(g->map, node + 1);

	pos_stime = g->log->next_value_pos(g->nodes + timestart + 1, startnode, endnode);
	if (pos_stime > endnode) {
		pos_stime = endnode;
	}

	pos_etime = g->log->next_value_pos(g->nodes + timeend + 1, startnode, endnode);
	if (pos_etime > endnode) {
		pos_etime = endnode;
	}
	//printf("stime: %u\netime: %u\n", pos_stime, pos_etime);
	*buffer = 0;
  vector<uint> rng;
	g->log->range_report(pos_stime, pos_etime, 0, g->nodes , rng);
	//count_symbols_range(g->log, startnode, endnode, buffer2);

#ifdef DEBUG
	printf("wt_range_report: "); print_arraysort(res);
#endif
	//printf("wt_range_report: "); print_arraysort(buffer2);
	j = 0;
	for (i = 0; i < rng.size(); i += 2) {
		buffer[++j] = rng[i];
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
	//uint *buffer;
  vector<uint> buffer;
	uint nextnode;
	
  //buffer = malloc(sizeof(uint)*BUFFER);
	//*buffer = 0;
  
	g->log->select_all(node, buffer);
  
	j = 0;
	for (i = 0; i < buffer.size(); i++) {
		curr_node = belong(g->map, buffer[i]);

		startnode = start(g->map, curr_node);
		nextnode = start(g->map, curr_node + 1);
    
		endnode_log = g->log->next_value_pos(g->nodes + time + 1, startnode, nextnode);
		endnode = nextnode;
		if (endnode_log < endnode) {
			endnode = endnode_log;
		}

		cant = i;
		while(i < buffer.size() && buffer[i] < endnode ) {
			i++;
		}
		cpos = i;

		if ( (cpos - cant) % 2 == 1 ) {
			res[++j] = curr_node;
		}

		while(i < buffer.size() && buffer[i] < nextnode) {
			i++;
		}
		i--;

	}
	*res = j;

	//free(buffer);
}



void get_reverse_point_slow(uint *res, struct tgs *g, uint node, uint time) {
	uint last_node, curr_node;
	uint i, j;
	uint cant, cpos;
	uint startnode, endnode, endnode_log;
  
  vector<uint> buffer;
  
	//uint *buffer;
  //buffer = malloc(sizeof(uint)*BUFFER);
  //*buffer = 0;

	g->log->select_all(node, buffer);

	last_node = UINT_MAX;

	j = 0;

	for (i = 0; i < buffer.size(); i++) {
		curr_node = belong(g->map, buffer[i]);

#ifdef DEBUG
		printf("curr_node: %u\n", curr_node);
#endif

		if ( curr_node != last_node ) {
			startnode = start(g->map, curr_node);
			endnode = start(g->map, curr_node + 1);

			endnode_log = g->log->next_value_pos(g->nodes + time + 1, startnode, endnode);
			if (endnode_log < endnode) {
				endnode = endnode_log;
			}
#ifdef DEBUG
			printf("startnode: %u\nendnode: %u\n", startnode, endnode);
#endif
			cant = g->log->rank(node, startnode);
			cpos = g->log->rank(node, endnode);

			//printf("cant: %u\ncpos: %u\n", cant,cpos);

			if ( (cpos - cant) % 2 == 1 ) {
				res[++j] = curr_node;
			}
		}
		last_node = curr_node;
	}
	*res = j;

	//free(buffer);
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
	//uint *buffer;
	uint nextnode;


  vector<uint> buffer;

	//buffer = malloc(sizeof(uint)*BUFFER);

	//*buffer = 0;

	g->log->select_all(node, buffer);

	last_node = UINT_MAX;

	j = 0;

	for (i = 0; i < buffer.size(); i++) {
		curr_node = belong(g->map, buffer[i]);

		startnode = start(g->map, curr_node);
		nextnode = start(g->map, curr_node + 1);

		endnode = nextnode;

		pos_stime = g->log->next_value_pos(g->nodes + ts + 1, startnode, endnode);
		if (pos_stime > endnode) {
			pos_stime = endnode;
		}

		pos_etime = g->log->next_value_pos(g->nodes + te + 1, startnode, endnode);
		if (pos_etime > endnode) {
			pos_etime = endnode;
		}

   // printf("startnode: %u\nendnode: %u\n", startnode, endnode);
    //printf("pos_stime: %u\npos_etime: %u\n", pos_stime, pos_etime);

#ifdef DEBUG
		printf("startnode: %u\nendnode: %u\n", startnode, endnode);
#endif
		//cstart = rank_wt(g->log, node, startnode);
		cstart = i;
    //printf("rankwt: %u\n", g->log->rank(node, startnode));
    //printf("cstart: %u\n", cstart);



		//cts = rank_wt(g->log, node, pos_stime);
		while(i < buffer.size() && buffer[i] < pos_stime ) {
					i++;
		}
		cts = i;

    //printf("rankwt: %u\n", g->log->rank(node, pos_stime));
    //printf("cts: %u\n", cts);


		//cte = rank_wt(g->log, node, pos_etime);
		while(i < buffer.size() && buffer[i] < pos_etime ) {
			i++;
		}
		cte = i;

    //printf("rankwt: %u\n", g->log->rank(node, pos_etime));
    //printf("cte: %u\n", cte);



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

		while(i < buffer.size() && buffer[i] < nextnode) {
			i++;
		}
		i--;


	}
	*res = j;

	//free(buffer);
}


void get_reverse_interval_slow(uint *res, struct tgs *g, uint node, uint ts, uint te, uint semantic) {
	uint last_node, curr_node;
	uint i, j;
	uint cstart, cts, cte;
	uint startnode, endnode, pos_stime, pos_etime;
	//uint *buffer;

//	buffer = malloc(sizeof(uint)*BUFFER);

	//*buffer = 0;

  vector<uint> buffer;

	g->log->select_all(node, buffer);

	last_node = UINT_MAX;

	j = 0;

	for (i = 0; i < buffer.size(); i++) {
		curr_node = belong(g->map, buffer[i]);

#ifdef DEBUG
		printf("curr_node: %u\n", curr_node);
#endif

		if ( curr_node != last_node ) {
			startnode = start(g->map, curr_node);
			endnode = start(g->map, curr_node + 1);

			pos_stime = g->log->next_value_pos(g->nodes + ts + 1, startnode, endnode);
			if (pos_stime > endnode) {
				pos_stime = endnode;
			}

			pos_etime = g->log->next_value_pos(g->nodes + te + 1, startnode, endnode);
			if (pos_etime > endnode) {
				pos_etime = endnode;
			}



#ifdef DEBUG
			printf("startnode: %u\nendnode: %u\n", startnode, endnode);
#endif
			cstart = g->log->rank(node, startnode);
			cts = g->log->rank(node, pos_stime);
			cte = g->log->rank(node, pos_etime);

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

	//free(buffer);
}

