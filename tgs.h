/*
 * tgs.h
 *
 *  Created on: Mar 20, 2013
 *      Author: diegocaro
 */

#ifndef TGS_H_
#define TGS_H_

#include "bitrankw32int.h"
#include "wt.h"

#define BUFFER 1024*1024

struct tgs {
	uint size_log;
	uint size_map;
	uint nodes;
	uint changes;
	uint maxtime;
	struct wt *log;
	bitRankW32Int *map;

};


void tgs_save(struct tgs *a, FILE *f);
void tgs_free(struct tgs *a);
void tgs_load(struct tgs *a, FILE *f);


inline uint start(bitRankW32Int *b, uint i) ;
inline uint belong(bitRankW32Int *b, uint i);

void get_neighbors_point(uint *res, struct tgs *g, uint node, uint time);
void get_neighbors_interval(uint *res, struct tgs *g, uint node, uint timestart, uint timeend, uint semantic);
void get_neighbors_weak(uint *res, struct tgs *g, uint node, uint timestart, uint timeend);
void get_neighbors_strong(uint *res, struct tgs *g, uint node, uint timestart, uint timeend);

void get_reverse_point(uint *res, struct tgs *g, uint node, uint time);

void get_reverse_interval(uint *res, struct tgs *g, uint node, uint ts, uint te, uint semantic);
void get_reverse_weak(uint *res, struct tgs *g, uint node, uint ts, uint te);
void get_reverse_strong(uint *res, struct tgs *g, uint node, uint ts, uint te);


void get_reverse_point_slow(uint *res, struct tgs *g, uint node, uint time);

#endif /* TGS_H_ */
