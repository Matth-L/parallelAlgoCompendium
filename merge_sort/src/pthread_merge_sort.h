/******************************************************************************* 
 * @file pthread_merge_sort.h 
 *******************************************************************************/

#ifndef PTHREAD_MERGE_SORT_H
#define PTHREAD_MERGE_SORT_H

#include <pthread.h>
#include <limits.h>

typedef struct Thread_data {
    int n;
    int *tab;
    int depth; // Depth parameter to control threading depth
} data_t;

extern int max_threads;

int log2floor(int n);
void pretty_print_array(int *tab, int n);
void fusion_pthread(data_t u, data_t v, int *T);
void *tri_fusion_pthread(void *arg);

#endif 
