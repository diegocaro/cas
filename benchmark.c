/*
 * This code was originally made by Guillermo de Bernardo <guillermo.debernardo@udc.es>
 *
 * It's evaluate the performance of the datastructure.
 *
 * Has been modified by Diego Caro <diegocaro@gmail.com>
 */
#include<stdio.h>
#include<time.h>
#include<stdlib.h>

#include "timing.h"
#include "tgs.h"
#include "debug.h"
#include "arraysort.h"

//#define EXPERIMENTS 1

/*
FIXME: Non soporta un nœmero de nodos maior que MAX_INT.

As consultas son nodos aleatorios (non Ž unha permutaci—n,
pode haber repetidos)
*/


#define CELL 6
#define CELL_WEAK 7
#define CELL_STRONG 8

#define DIRECT_NEIGHBORS 0
#define REVERSE_NEIGHBORS 1
#define DIRECT_NEIGHBORS_WEAK 2
#define DIRECT_NEIGHBORS_STRONG 3
#define REVERSE_NEIGHBORS_WEAK 4
#define REVERSE_NEIGHBORS_STRONG 5

#define ISIZE 5

int compareRes(unsigned int * l1, unsigned int * l2) {
        int i;
        if (l1[0]!=l2[0]) return 1;
        for (i = 1; i <= l1[0]; i++) {
                if (l1[i] != l2[i])  {
                        printf("got value %u at index %d, expected %u\n", l1[i], i, l2[i]);
                        return 1;
                }
        }
        return 0;
}

typedef struct squery {
        int type;
        unsigned int row;
        unsigned int column;
        unsigned int time; //point
        unsigned int initime; //interval
        unsigned int endtime; //interval
        unsigned int expectednres;
        unsigned int * expectedres;
} TimeQuery;

#define MAX_QUERIES 10000
TimeQuery * readQueries(char * filename, int * nqueries) {
        TimeQuery * ret = (TimeQuery *) malloc(MAX_QUERIES*sizeof(TimeQuery));
        FILE * queryFile = fopen(filename, "r");
        int curn = 0;
        while(1) {
                TimeQuery *query = &ret[curn];
                int res = EOF;
                res = fscanf(queryFile, "%d", &query->type);
                if (res == EOF) break;
                switch(query->type) {
                        case CELL: {
                                res = fscanf(queryFile, "%d %d %d\n", &query->row, &query->column, &query->time);
                                break;
                        }
                        case CELL_WEAK: case CELL_STRONG: {
                                res = fscanf(queryFile, "%d %d %d %d\n", &query->row, &query->column, &query->initime, &query->endtime);
                                break;
                        }
                        case DIRECT_NEIGHBORS: case REVERSE_NEIGHBORS: {
                                res = fscanf(queryFile, "%d %d\n", &query->row, &query->time);
                                break;
                        }
                        case DIRECT_NEIGHBORS_WEAK: case DIRECT_NEIGHBORS_STRONG:
                        case REVERSE_NEIGHBORS_WEAK: case REVERSE_NEIGHBORS_STRONG: {
                                res = fscanf(queryFile, "%d %d %d\n", &query->row, &query->initime, &query->endtime);
                                break;
                        }
                }

                if(query->type  == CELL || query->type == CELL_STRONG || query->type == CELL_WEAK)
                        res = fscanf(queryFile, "%d\n", &query->expectednres);
                else {
                        res = fscanf(queryFile, "%d", &query->expectednres);
                        int i;
                        query->expectedres = (unsigned int *) malloc((1+query->expectednres)*sizeof(unsigned int));
                        query->expectedres[0] = query->expectednres;
                        for (i = 1; i <= query->expectednres; i++) {
                                fscanf(queryFile, "%u ", &query->expectedres[i]);
                        }
                }
                curn++;
        }
        *nqueries = curn;
        return ret;
}

void printQuery(TimeQuery q) {
        printf("%d %d %d %d %d %d %d\n", q.type, q.row, q.column, q.time, q.initime, q.endtime, q.expectednres);
}

#define CHECK_RESULTS 1
int savegotFile = 0;

int main(int argc, char ** argv) {

        char * fileName;
        struct tgs index;
        int totalres = 0;
        char * gotqueryFile = NULL;
        FILE * gotFile = NULL;
        FILE * f;
        unsigned int * gotreslist;
        int gotres = 0;

        if (argc < 3) {
                printf("Usage: %s <graphfile> <queryfile> [<gotqueryfile>]", argv[0]);
                exit(1);
        }
        if (argc == 4) {
          savegotFile = 1;
          gotqueryFile = argv[3];
        }

        fileName = argv[1];

        f = fopen(fileName, "r");
        tgs_load(&index, f);
        fclose(f);

        gotreslist = malloc(sizeof(unsigned int)*BUFFER);

        int nqueries = 0;
        TimeQuery * queries = readQueries(argv[2], &nqueries);

        int i;

        startTimer();

        clock_t initime = clock();
        for (i = 0; i < nqueries; i++) {
                TimeQuery query = queries[i];


                switch(query.type) {
//                case CELL: {
//                        gotres = findEdge(tree, query.row, query.column, query.time);
//                        break;
//                }
//                case CELL_WEAK: {
//                        gotres = findEdgeInterval(tree, query.row, query.column, query.initime, query.endtime, 0);
//                        break;
//                }
//                case CELL_STRONG: {
//                        gotres = findEdgeInterval(tree, query.row, query.column, query.initime, query.endtime, 1);
//                        break;
//                }
                case DIRECT_NEIGHBORS: {
                        get_neighbors_point(gotreslist, &index, query.row, query.time);
                        break;
                }
                case REVERSE_NEIGHBORS: {
                        get_reverse_point(gotreslist, &index, query.row, query.time);
                        break;
                }
                case DIRECT_NEIGHBORS_WEAK: {
                        get_neighbors_weak(gotreslist, &index, query.row, query.initime, query.endtime);
                        break;
                }
                case REVERSE_NEIGHBORS_WEAK: {
                        get_reverse_weak(gotreslist, &index, query.row, query.initime, query.endtime);
                        break;
                }
                case DIRECT_NEIGHBORS_STRONG: {
                        get_neighbors_strong(gotreslist, &index, query.row, query.initime, query.endtime);
                        break;
                }
                case REVERSE_NEIGHBORS_STRONG: {
                        get_reverse_strong(gotreslist, &index, query.row, query.initime, query.endtime);
                        break;
                }
//              case FULLRANGE: {
//                      gotres = findRange(tree, 0, tree->nNodesReal, 0, tree->nNodesReal, time)[0][0];
//                      break;
//              }
                }

#ifndef EXPERIMENTS
//                //Comentar para medir tiempos:
                if (CHECK_RESULTS) {
                  if (savegotFile) {
                    gotFile = fopen(gotqueryFile, "a");
                    switch(query.type) {
                    case CELL: {
                      fprintf(gotFile, "%d %d %d\n", query.row, query.column, query.time);
                      break;
                    }
                    case CELL_WEAK: case CELL_STRONG: {
                      fprintf(gotFile, "%d %d %d %d\n", query.row, query.column, query.initime, query.endtime);
                      break;
                    }
                    case DIRECT_NEIGHBORS: case REVERSE_NEIGHBORS: {
                      fprintf(gotFile, "%d %d\n", query.row, query.time);
                      break;
                    }
                    case DIRECT_NEIGHBORS_WEAK: case DIRECT_NEIGHBORS_STRONG:
                    case REVERSE_NEIGHBORS_WEAK: case REVERSE_NEIGHBORS_STRONG: {
                      fprintf(gotFile, "%d %d %d\n", query.row, query.initime, query.endtime);
                      break;
                    }
                    }

                    if (query.type == CELL || query.type == CELL_WEAK || query.type == CELL_STRONG) {
                      fprintf(gotFile,"0\n");
                    } else {
                      int j;
                      fprintf(gotFile, "%d", gotreslist[0]);
                      for (j = 1; j <= gotreslist[0]; j++) {
                        fprintf(gotFile, " %d", gotreslist[j]);
                      }
                      fprintf(gotFile, "\n");
                    }

                    fclose(gotFile);
                  }


                  int failcompare = 0;
                  if (query.type == CELL || query.type == CELL_WEAK || query.type == CELL_STRONG) {
                    failcompare = (gotres != query.expectednres);
                  } else {
                    failcompare = compareRes(gotreslist, query.expectedres);
                    gotres = gotreslist[0];
                  }
                  if (failcompare) {
                    printf("query queryType=%d, row=%d, column=%d, time=%d, initime=%d, endtime=%d, expectedres=%d\n", query.type, query.row, query.column, query.time, query.initime, query.endtime, query.expectednres);
                    printf("count: got %d expected %d\n", gotres, query.expectednres);
                    printf("expected: "); print_arraysort(query.expectedres);
                    printf("got     : "); print_arraysort(gotreslist);
                    exit(1);
                  }
                  totalres += gotres;
                }

#else
                totalres += *gotreslist;
#endif




        }
        clock_t endtime = clock();

        double difftime = (endtime - initime)*1000000/CLOCKS_PER_SEC;

//	printf("time = (%lf), %d queries, %lf micros/query, %lf micros/arista\n",
//	               difftime, nqueries,
//	               difftime/nqueries, difftime/totalres);

	        printf("time = %lf (%ld) (%lf), %d queries, %lf micros/query, %lf micros/arista\n",
	               timeFromBegin(), realTimeFromBegin(), difftime, nqueries,
	               difftime/nqueries, difftime/totalres);

        //destroyK2Tree(tree);

        exit(0);
}
