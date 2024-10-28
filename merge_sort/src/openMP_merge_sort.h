/*******************************************************************************
 * @file openMP_merge_sort.h
 * @brief Header file for OpenMP-based Parallel Merge Sort
 ******************************************************************************/

#ifndef OPENMP_MERGE_SORT_H
#define OPENMP_MERGE_SORT_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <omp.h>

// Global variables
extern int max_threads;
extern int depth; // Tracks current recursion depth for controlling threading

// Function declarations
int log2floor(int n);
void pretty_print_array(int *tab, int n);
void fusion_openmp(int *U, int n, int *V, int m, int *T);
void tri_fusion_openmp(int *tab, int n);

#endif // OPENMP_MERGE_SORT_H
