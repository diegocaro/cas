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
#include <cstring>
#include "debug.h"

#include <libcdsBasics.h>
#include <BitSequence.h>

#include <wavelet_matrix.h>

#include "tgs.h"


#include <map>
#include <vector>

#define RANK_FACTOR 20
#define DEFAULT_SAMPLING 32

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

enum bitseq {
	RG, R3,
};

struct opts {
	enum bitseq bs; //bit data structure
	char *outfile;
	//char *infile;
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

	//printf("map: %X\n", a->map[0]);
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

	//printf("map: %X\n", a->map->data[0]);
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

void readbin( char *filename, struct adjlog *adjlog) {
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
                //printf("%d\n",k);
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

void readcontacts(struct adjlog *adjlog) {
	uint nodes, edges, lifetime, contacts;
	uint u,v,a,b;

	vector < map<uint, vector<uint> > > btable;

	scanf("%u %u %u %u", &nodes, &edges, &lifetime, &contacts);

	for(uint i = 0; i < nodes; i++) {
		map<uint, vector<uint> > t;
		btable.push_back(t);
	}

	uint c_read = 0;
	while( EOF != scanf("%u %u %u %u", &u, &v, &a, &b)) {
		c_read++;
		if(c_read%500000==0) fprintf(stderr, "Processing %.1f%%\r", (float)c_read/contacts*100);

		btable[u][a].push_back(v);
		if (b == lifetime-1) continue;

		btable[u][b].push_back(v);
	}
	fprintf(stderr, "Processing %.1f%%\r", (float)c_read/contacts*100);
	assert(c_read == contacts);

	uint lenS = 4*contacts; //upper bound

	uint *S = new uint[lenS];
	uint p = 0;


	uint sizeB = uint_len(lenS+nodes,1);
	uint *B = (uint *)calloc(sizeB, sizeof(uint));
	//uint q = 0;

	map<uint, vector<uint> >::iterator it;

	for(uint i = 0; i < nodes; i++) {
		bitset(B, i+p);
		for( it = btable[i].begin(); it != btable[i].end(); ++it) {
			S[p++] = nodes+it->first;
			for(uint j = 0; j < (it->second).size(); j++ ) {
				S[p++] = (it->second).at(j);
			}

		}

	}

	adjlog->changes = 2*contacts;
	adjlog->maxtime = lifetime;
	adjlog->nodes = nodes;

	adjlog->size_log = p;
	adjlog->log = S;

	adjlog->map = B;
	adjlog->size_map = nodes+p;

}

void create_index(struct tgs *tgs, struct adjlog *adjlog, struct opts *opts) {
	BitSequenceBuilder *bs;
	switch(opts->bs) {
		case RG:
		bs = new BitSequenceBuilderRG(RANK_FACTOR); // by default, 5% of extra space for bitmaps
		break;
		case R3:
		bs = new BitSequenceBuilderRRR(DEFAULT_SAMPLING); // DEFAULT_SAMPLING for RRR is 32 
		break;
	}
	tgs->nodes = adjlog->nodes;
	tgs->changes = adjlog->changes;
	tgs->maxtime = adjlog->maxtime;
	
	tgs->size_log = adjlog->size_log;
	tgs->size_map = adjlog->size_map;
	//	tgs->map = createBitRankW32Int(adjlog->map, tgs->size_map, 0, RANK_FACTOR);
	tgs->map = bs->build(adjlog->map, tgs->size_map);
	
	//printf("maxtime: %u\nnodes: %u\nmaxcosa: %u\n", tgs->maxtime, tgs->nodes, tgs->nodes + tgs->maxtime);
	//creating wavelet_tree
	//uint next_power;
	//next_power = (uint) log2(tgs->nodes + tgs->maxtime) + 1;
	//next_power = (uint) 1 << next_power;
	//LOG("next power: %u\n", next_power);
	INFO("Creating WT for log\n");

	//tgs->log = (struct wt*)malloc(sizeof(struct wt));
	//create_wt(&adjlog->log, tgs->size_log, tgs->log, 0, next_power);
  
	/*
	tgs->log = new WaveletTree(adjlog->log, adjlog->size_log, 
	new wt_coder_binary(adjlog->log, adjlog->size_log,	new MapperNone()),
	bs, 
	new MapperNone());
	*/
	
	tgs->log = new WaveletMatrix(adjlog->log, adjlog->size_log, bs);
}

int readopts(int argc, char **argv, struct opts *opts) {
	int o;
	
	
	// Default options
	opts->bs = RG;

	while ((o = getopt(argc, argv, "b:")) != -1) {
		switch (o) {
			case 'b':
			if(strcmp(optarg, "RG")==0) {
				INFO("Using RG for bitmaps");
				opts->bs = RG;
			}
			else if(strcmp(optarg, "RRR")==0) {
				INFO("Using RRR for bitmaps");
				opts->bs = R3;
			}
			break;
			default: /* '?' */
			break;
		}
	}
	
        if (optind >= argc || (argc-optind) < 1 ) {
		fprintf(stderr, "%s [-b RG,RRR] <outputfile>\n", argv[0]);
		fprintf(stderr, "Expected argument after options\n");
		exit(EXIT_FAILURE);
        }
	
	//opts->infile = argv[optind];
	opts->outfile = argv[optind]; //era: optind +1
	
	return optind;

}


int main( int argc, char *argv[]) {
	struct adjlog tg;
	struct tgs tgindex;
	struct opts opts;
	
	readopts(argc, argv, &opts);
	
	//readbin(opts.infile, &tg);
	readcontacts(&tg);
	
	printadjlog(&tg);
	
	create_index(&tgindex, &tg, &opts);
	
	printtgs(&tgindex);

	ofstream f;
	f.open(opts.outfile, ios::binary);
	tgs_save(&tgindex, f);
	f.close();
	
	//adjlog_free(&tg);
	//tgs_free(&tgindex);
	
	return 0;
}
