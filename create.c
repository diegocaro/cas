/*
 * create.c
 *
 *  Created on: Mar 11, 2013
 *      Author: diegocaro
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <sys/types.h> //uint
#include "debug.h"
#include "adjlog.h"
#include "basic.h"
#include "bitrankw32int.h"
#include "wt.h"

#include "tgs.h"

#define RANK_FACTOR 20

// Print unsigned integer in binary format with spaces separating each byte.
void print_binary(unsigned int num) {
  int arr[32];
  int i = 0;
  while (i++ < 32 || num / 2 != 0) {
    arr[i - 1] = num % 2;
    num /= 2;
  }

  for (i = 31; i >= 0; i--) {
    printf("%d", arr[i]);
    if (i % 8 == 0)
      printf(" ");
  }
}


void adjlog_free(struct adjlog *a) {

	free(a->log);
	a->log = NULL;
	
	free(a->map);
	a->map = NULL;
}

void printadjlog(struct adjlog *a) {
	uint i;


	printf("Nodes: %u\n", a->nodes);
	printf("Changes: %u\n", a->changes);
	printf("Maxtime: %u\n", a->maxtime);
	printf("Size: %u\n", a->size_log);

	printf("map: %X\n", a->map[0]);
	print_binary(a->map[0]);


	for(i = 0; i < a->size_log; i++) printf(" %u", a->log[i]);


	uint node = 0;
	uint j;
	for ( j = 0; j < a->size_map; j++) {
//		printf("bitget %d\n",  bitget(a->map, i));
		if ( bitget(&a->map[0], j) == 1) {
			//its a node
			printf("\n%u: ", node);
			node++;
		}
		else {
		i = j-node;

		if ( a->log[i] < a->nodes) {
			//its an edge
			printf(" +%u", a->log[i]);
		}
		else {
			//its time
			printf(" *%u (%u)", a->log[i] - a->nodes, a->log[i]);
		}
	}
		
	}
	
	printf("\n");
}


void printtgs(struct tgs *a) {
	uint i;


	printf("Nodes: %u\n", a->nodes);
	printf("Changes: %u\n", a->changes);
	printf("Maxtime: %u\n", a->maxtime);
	printf("Size: %u\n", a->size_log);

	printf("map: %X\n", a->map->data[0]);
	print_binary(a->map->data[0]);


	for(i = 0; i < a->size_log; i++) printf(" %u", access_wt(a->log, i));


	uint node = 0;
	uint j;
	uint curr;
	for ( j = 0; j < a->size_map; j++) {
//		printf("bitget %d\n",  bitget(a->map, i));
		if ( isBitSet(a->map, j) == 1) {
			//its a node
			printf("\n%u: ", node);
			node++;
		}
		else {
		i = j-node;

		curr = access_wt(a->log, i);
		if ( curr < a->nodes) {
			//its an edge
			printf(" +%u", curr);
		}
		else {
			//its time
			printf(" *%u (%u)", curr - a->nodes, curr);
		}
	}

	}

	printf("\n");
}

void create( char *filename, struct adjlog *adjlog) {
	uint nodes;
	uint changes;
	uint maxtime;
	uint *log;
	uint *map_nodes;
	uint size_map;
	uint *p;
	uint size;
	
	uint from, to, time, op;
	uint last_time, last_from;
	
	FILE *f;
	
	f = fopen(filename, "r");

	fscanf(f, "%u %u %u", &nodes, &changes, &maxtime);

	
	size_map = enteros(nodes+changes,1);
	map_nodes = (uint *) calloc( size_map, sizeof(uint));
	
	printf("you need %u integer for %u map_nodes\n", size_map, nodes+changes);
	
	log = (uint *)malloc( sizeof(int) * (2*changes));
	p = log;
	
	last_time = UINT_MAX;
	last_from = UINT_MAX;

	while ( EOF != fscanf(f, "%u %u %u %u", &from, &to, &time, &op)) {
		if ( from != last_from) {
			
			// if next change belongs from the previous node
			if ( last_from +1 == from) {
				// *p++ = nodes + from;
				bitset(map_nodes, from + (p-log));
				// (p - log) are the current number of items in the adjacency log
			}
			else {
				while (++last_from <= from) {
					// *p++ = nodes + last_from;
					bitset(map_nodes, last_from + (p-log));
					// (p - log) are the current number of items in the adjacency log
				}
			}
			
			last_from = from;
			last_time = UINT_MAX;
		}
		
		if ( time != last_time) {
			*p++ = nodes + time;
		}
		
		*p++ = to;
	}
	
	
	fclose(f);
	
	
	size = p - log;
	printf("p - log: %lu\n", p-log);
	printf("log : %p", log);
	log =  realloc(log,  sizeof(uint) * size);
	printf("log : %p", log);
	
	
	map_nodes =  realloc(map_nodes, sizeof(uint) * enteros(size+nodes,1));

	adjlog->nodes = nodes;
	adjlog->changes = changes;
	adjlog->maxtime = maxtime;
	adjlog->size_log = size;
	adjlog->log = log;
	adjlog->map = map_nodes;
	adjlog->size_map = size + nodes;
	
}



void create_index(struct tgs *tgs, struct adjlog *adjlog) {
	uint next_power;
	tgs->nodes = adjlog->nodes;
	tgs->changes = adjlog->changes;
	tgs->maxtime = adjlog->maxtime;
	
	tgs->size_log = adjlog->size_log;
	tgs->size_map = adjlog->size_map;
	tgs->map = createBitRankW32Int(adjlog->map, tgs->size_map, 0, RANK_FACTOR);
	
	printf("maxtime: %u\nnodes: %u\nmaxcosa: %u\n", tgs->maxtime, tgs->nodes, tgs->nodes + tgs->maxtime);
	//creating wavelet_tree
	next_power = (uint) log2(tgs->nodes + tgs->maxtime) + 1;
	next_power = (uint) 1 << next_power;
	LOG("next power: %u\n", next_power);
 	INFO("Creating WT for log\n");

 	tgs->log = (struct wt*)malloc(sizeof(struct wt));
	create_wt(&adjlog->log, tgs->size_log, tgs->log, 0, next_power);
	
}

int main( int argc, char *argv[]) {
	struct adjlog tg;
	struct tgs tgindex;
	if (argc < 3) {
		fprintf(stderr, "%s <dataset> <outputfile>\n", argv[0]);
		exit(-1);
	}
	
	create(argv[1], &tg);
	
	
	printadjlog(&tg);
	
	create_index(&tgindex, &tg);
	
	printtgs(&tgindex);

	FILE *f;
	f = fopen(argv[2], "w");
	tgs_save(&tgindex, f);
	fclose(f);
	
	adjlog_free(&tg);
	tgs_free(&tgindex);
	
	return 0;
	/*
	//uint mem = atoi(argv[2]);
	//uint K = atoi(argv[3]);
	
	//RePair *rc = new RePair( tg.log, tg.size, mem, argv[1], K);
	//rc->compress(tg.size);
	//rc->save(tg.size);
	
	size_t csymbols;
	size_t crules;
	Tdiccarray *rules;
	
	IRePair rp;
	rp.compress( tg.log, tg.size, &csymbols, &crules, &rules);
	
	
	printf("csymbols: %u\ncrules: %u\n", csymbols, crules);
	//printf("alph: %d\nn: %d\n", rp.alph, rp.n);
	
	FILE *f;
	
	char filename[255];
	
	// saving compressed sequence
	sprintf(filename, "%s.c", argv[1]);
	f = fopen(filename, "w");
	rp.save_compressed(f);
	fclose(f);
	
	// saving rules
	sprintf(filename, "%s.r", argv[1]);
	f = fopen(filename, "w");
	rp.save_rules(f);
	fclose(f);
	
	
	// savint metadata
	struct rpadjlog rplog;
	rplog.nodes = tg.nodes;
	rplog.changes = tg.changes;
	rplog.maxtime = tg.maxtime;
	rplog.size = tg.size;
	
	rplog.csize = rp.get_csize();
	rplog.frule = csymbols;
	rplog.nrules = crules;
	
	sprintf(filename, "%s.info", argv[1]);
	f = fopen(filename, "w");
	fwrite(&rplog, sizeof(struct rpadjlog), 1, f);
	fclose(f);
	
	
	
	
	rp.stats();
	
	
	
	return 0;*/
}
