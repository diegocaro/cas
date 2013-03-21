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

#endif /* TGS_H_ */
