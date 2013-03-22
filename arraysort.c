/*
 * arraysort.c
 *
 *  Created on: Mar 22, 2013
 *      Author: diegocaro
 */

#include <stdio.h>

void print_arraysort(unsigned int *a) {
  int i;

  for (i = 1; i <= *a; i++) {
    printf("%d ", a[i]);
  }
  printf("\n");

}
