/*
 * tgs.c
 *
 *  Created on: Mar 20, 2013
 *      Author: diegocaro
 */

#include <stdio.h>

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


