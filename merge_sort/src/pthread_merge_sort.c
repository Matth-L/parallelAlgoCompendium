/*******************************************************************************
 * @file pthread_merge_sort.c
 * @brief Parallel Merge Sort with Optimized Thread Spawning and Memory Usage
 ******************************************************************************/
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>
#include <omp.h>
#include <semaphore.h>
#include "utils.h"

int max_threads;

typedef struct Thread_data
{
    int n;
    int *tab;
    int depth; // New depth parameter to control threading depth
} data_t;

void fusion_pthread(data_t u, data_t v, int *T)
{
    int i = 0, j = 0;
    int n = u.n;
    int m = v.n;
    u.tab[n] = INT_MAX;
    v.tab[m] = INT_MAX;
    for (int k = 0; k < m + n; k++)
    {
        if (u.tab[i] < v.tab[j])
        {
            T[k] = u.tab[i++];
        }
        else
        {
            T[k] = v.tab[j++];
        }
    }
}

void *tri_fusion_pthread(void *arg)
{
    data_t *t = (data_t *)arg;
    if (t->n < 2)
        return NULL;

    int mid = t->n / 2;

    data_t u = {mid, malloc((mid + 1) * sizeof(int)), t->depth + 1};
    data_t v = {t->n - mid, malloc((t->n - mid + 1) * sizeof(int)), t->depth + 1};

    
    for (int i = 0; i < mid; i++)
    {
        u.tab[i] = t->tab[i];
        v.tab[i] = t->tab[i+mid];
    }
    
    pthread_t child;
    if (t->depth < log2floor(max_threads)) // Limit threading to top 3 levels
    {
        if (pthread_create(&child, NULL, tri_fusion_pthread, &u) != 0)
        {
            perror("pthread_create error");
            exit(EXIT_FAILURE);
        }
        tri_fusion_pthread(&v);
        pthread_join(child, NULL);
    }
    else // Sequential merge sort for deeper recursion
    {
        tri_fusion_pthread(&u);
        tri_fusion_pthread(&v);
    }

    fusion_pthread(u, v, t->tab);

    free(u.tab);
    free(v.tab);
    return NULL;
}
