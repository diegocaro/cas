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

#include <libcdsBasics.h>
#include <BitSequence.h>
#include <Mapper.h>
#include <Sequence.h>

#include "tgs.h"

#define RANK_FACTOR 20

using namespace std;
using namespace cds_static;

struct infolog {
	uint nodes;
	uint changes;
	uint maxtime;
	uint size;
};

struct adjlog {
	uint nodes; //number of nodes from [0 ... nodes -1]
	uint changes; //total number of changes
	uint maxtime; //maximum time in the dataset from [0 ... maxtime-1]

	uint size_log; //size of the log
	uint *log; //including time and edges

	uint *map; // bitmap of position of nodes
	uint size_map;
};

// Print unsigned integer in binary format with spaces separating each byte.
void print_binary(unsigned int num) {
  int arr[32];
  int i = 0;
  while (i++ < 32 || num / 2 != 0) {
    arr[i - 1] = num % 2;
    num /= 2;
  }

  for (i = 0; i < 32; i++) {
    printf("%d", arr[i]);
    if (i % 8 == 0 && i != 0)
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
	//print_binary(a->map[0]);
	//for(i = 0; i < a->size_log; i++) printf(" %u", a->log[i]);

	printf("\n\n");
	for(i = 0; i < 10; i++) printf(" %u", a->log[i]);
	printf("\n\n");
	uint node = 0;
	uint j;
	for ( j = 0; j < a->size_map; j++) {
		if (j > 10) return;

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
	//print_binary(a->map->data[0]);
	//for(i = 0; i < a->size_log; i++) printf(" %u", access_wt(a->log, i));

	printf("\n\n");
	for(i = 0; i < 10; i++) printf(" %u", a->log->access(i));
	printf("\n\n");

	uint node = 0;
	uint j;
	uint curr;
	for ( j = 0; j < a->size_map; j++) {
		if (j > 10) return;
//		printf("bitget %d\n",  bitget(a->map, i));
		if ( a->map->access(j) == 1) {
			//its a node
			printf("\n%u: ", node);
			node++;
		}
		else {
		i = j-node;

		curr = a->log->access(i);
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
	uint *log;
	uint *p;
	uint i;
	uint size;

	//uint from, to, time, op;
	//uint last_time, last_from;

	FILE *f;
	int k;

	struct infolog infolog;
	uint size_map;
	uint *map_nodes;

	f = fopen(filename, "r");

	fread(&infolog, sizeof(struct infolog), 1, f);

	size_map = uint_len(infolog.size, 1);//(nodes+changes+32-1)/32;//enteros(nodes+changes,1);
	map_nodes = (uint *) calloc( size_map, sizeof(uint));


	//log = (uint *)calloc((infolog.size)* 1.02, sizeof(uint));
	log = (uint *)malloc( sizeof(uint) * infolog.size);
	printf("size of log: %u\n",2*infolog.changes);
	p = log;
	for(i = 0; i < infolog.size; i++) {
		fread(&k, sizeof(k), 1, f);

		if (k>=0) {
			*p++ = (uint)k;
		} else {
			//printf("k: %d in %d\n",k, i);
			// bitmap for
			//printf("from %u\tlast_from %u\n", from, last_from);
			//printf("setting %u bit (%u)\n", from + (p-log), nodes+2*changes);
			//printf("setting %u bit\n",i);
			bitset(map_nodes, i);

		}
	}


	fclose(f);

	size = p - log;
	printf("p - log: %lu\n", p-log);
	//printf("log : %p", log);
	log =  (uint *)realloc(log,  sizeof(uint) * size);
	//printf("log : %p", log);

	//map_nodes = realloc(map_nodes, sizeof(uint) * enteros(size+nodes,1));

	adjlog->changes = infolog.changes;
	adjlog->maxtime = infolog.maxtime;
	adjlog->nodes = infolog.nodes;

	adjlog->size_log = size;
	adjlog->log = log;

	adjlog->map = map_nodes;
	adjlog->size_map = infolog.size;

}



void create_index(struct tgs *tgs, struct adjlog *adjlog) {
	
	tgs->nodes = adjlog->nodes;
	tgs->changes = adjlog->changes;
	tgs->maxtime = adjlog->maxtime;
	
	tgs->size_log = adjlog->size_log;
	tgs->size_map = adjlog->size_map;
//	tgs->map = createBitRankW32Int(adjlog->map, tgs->size_map, 0, RANK_FACTOR);
  tgs->map = new BitSequenceRG(adjlog->map, tgs->size_map, RANK_FACTOR);
	
	//printf("maxtime: %u\nnodes: %u\nmaxcosa: %u\n", tgs->maxtime, tgs->nodes, tgs->nodes + tgs->maxtime);
	//creating wavelet_tree
	//uint next_power;
	//next_power = (uint) log2(tgs->nodes + tgs->maxtime) + 1;
	//next_power = (uint) 1 << next_power;
	//LOG("next power: %u\n", next_power);
 	INFO("Creating WT for log\n");

 	//tgs->log = (struct wt*)malloc(sizeof(struct wt));
	//create_wt(&adjlog->log, tgs->size_log, tgs->log, 0, next_power);
  
  tgs->log = new WaveletTree(adjlog->log, adjlog->size_log, 
				      new wt_coder_binary(adjlog->log, adjlog->size_log,	new MapperNone()),
				      new BitSequenceBuilderRG(RANK_FACTOR), 
				      new MapperNone());
	
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

	ofstream f;
	f.open(argv[2], ios::binary);
	tgs_save(&tgindex, f);
	f.close();
	
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
