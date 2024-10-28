/*******************************************************************************
 * @file sequential_merge_sort.h
 * @brief Header file for Sequential Merge Sort
 ******************************************************************************/

#ifndef SEQUENTIAL_MERGE_SORT_H
#define SEQUENTIAL_MERGE_SORT_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// Function declarations
void pretty_print_array(int *tab, int n);
void fusion_sequential(int *U, int n, int *V, int m, int *T);
void tri_fusion_sequential(int *tab, int n);

#endif // SEQUENTIAL_MERGE_SORT_H
