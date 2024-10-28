#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>
#include <omp.h>
#include <semaphore.h>
#include "utils.h"

int max_threads;
int depth;

void fusion_sequential(int *U, int n, int *V, int m, int *T)
{

    // procedure fusion(U[0..n-1],V[0..m-1],T[0..m-1+n-1])
    // i=j=0
    // U[n]=V[m]=∞
    // pour k=0 `a m-1+n-1 faire
    // si U[i]<V[j] alors
    // T[k]=U[i++]
    // sinon
    // T[k]=V[j++]
    int i = 0, j = 0;
    U[n] = INT_MAX;
    V[m] = INT_MAX;
    for (int k = 0; k < m + n; k++)
    {
        if (U[i] < V[j])
        {
            T[k] = U[i++];
        }
        else
        {
            T[k] = V[j++];
        }
    }
}

void tri_fusion_sequential(int *tab, int n)
{
    // procedure tri fusion(T[1..n])
    // si n est petit adhoc(T[1..n])
    // sinon
    // U[1..n/2]=T[1..n/2]
    // V[1..n/2]=T[1+n/2..n]
    // tri fusion(U)
    // tri fusion(V)
    // fusion(U,V,T)

    if (n < 2)
    {
        return;
    }

    int mid = n / 2;
    int *U = malloc(mid * sizeof(int));
    int *V = malloc((n - mid) * sizeof(int));

    for (int i = 0; i < mid; i++)
        U[i] = tab[i];
    for (int i = mid; i < n; i++)
        V[i - mid] = tab[i];

    tri_fusion_sequential(U, mid);
    tri_fusion_sequential(V, (n - mid));
    fusion_sequential(U, mid, V, (n - mid), tab);
}

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
        v.tab[i] = t->tab[i + mid];
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

void fusion_openmp(int *U, int n, int *V, int m, int *T)
{

    // procedure fusion_openmp(U[0..n-1],V[0..m-1],T[0..m-1+n-1])
    // i=j=0
    // U[n]=V[m]=∞
    // pour k=0 `a m-1+n-1 faire
    // si U[i]<V[j] alors
    // T[k]=U[i++]
    // sinon
    // T[k]=V[j++]
    int i = 0, j = 0;
    U[n] = INT_MAX;
    V[m] = INT_MAX;
    for (int k = 0; k < m + n; k++)
    {
        if (U[i] < V[j])
        {

            T[k] = U[i++];
        }
        else
        {
            T[k] = V[j++];
        }
    }
}

void tri_fusion_openmp(int *tab, int n)
{
    if (n < 2)
        return;

    int mid = n / 2;
    int *U = malloc((mid + 1) * sizeof(int));
    int *V = malloc((n - mid + 1) * sizeof(int));

    depth++;

    for (int i = 0; i < mid; i++)
    {
        U[i] = tab[i];
        V[i] = tab[i + mid];
    }

    if (depth < log2floor(max_threads)) // Limit threading to top 3 levels
    {
#pragma omp parallel sections
        {
#pragma omp section
            tri_fusion_openmp(U, mid);
#pragma omp section
            tri_fusion_openmp(V, (n - mid));
        }
    }
    else
    {
        tri_fusion_openmp(U, mid);
        tri_fusion_openmp(V, (n - mid));
    }

    fusion_openmp(U, mid, V, (n - mid), tab);
}

int main(int argc, char *argv[])
{

    int n_try = 10;
    double time_sequential;
    double tab_of_time_phtread[7];
    double tab_of_time_openmp[7];
    double tab_of_time_openmp2[7];

    int nb_threads[6] = {2, 4, 8, 16, 24, 48};

    for (int n_array_size = 2; n_array_size < 100000; n_array_size *= 2)
    {
        int T[n_array_size];
        for (int i = 0; i < n_array_size; i++)
        {
            T[i] = rand() % 100000;
        }

        /**********************************************
         * BENCHMARK SEQUENTIAL
         ***********************************************/

        for (int i = 0; i < n_try; i++)
        {
            double start = omp_get_wtime();
            tri_fusion_sequential(T, n_array_size);
            double stop = omp_get_wtime();
            time_sequential += stop - start;
        }
        time_sequential /= n_try;
        printf("\n");
        printf("\033[0;31mSize of array: %d\033[0m\n", n_array_size);
        printf("\033[0;34mAverage time for sequential: %f\033[0m\n", time_sequential);

        /**********************************************
         * BENCHMARK PTHREAD
         ***********************************************/
        data_t init_data = {n_array_size, T, 0};

        for (int i = 0; i < 6; i++)
        {
            for (int j = 0; j < n_try; j++)
            {
                max_threads = nb_threads[i];
                double start = omp_get_wtime();
                tri_fusion_pthread(&init_data);
                double stop = omp_get_wtime();
                tab_of_time_phtread[i] += stop - start;
            }
            tab_of_time_phtread[i] /= n_try;
        }
        printf("\033[0;32mAverage time for pthread: \033[0m\n");
        for (int i = 0; i < 6; i++)
        {
            printf("%f ", tab_of_time_phtread[i]);
        }
        printf("\n");
        /**********************************************
         * BENCHMARK OPENMP
         ***********************************************/

        for (int i = 0; i < 7; i++)
        {
            for (int j = 0; j < n_try; j++)
            {
                max_threads = nb_threads[i];
                depth = 0;
                double start = omp_get_wtime();
                tri_fusion_openmp(T, n_array_size);
                double stop = omp_get_wtime();
                tab_of_time_openmp[i] += stop - start;
            }
            tab_of_time_openmp[i] /= n_try;
        }
        printf("\033[0;33mAverage time for openmp: \033[0m\n");
        for (int i = 0; i < 6; i++)
        {
            printf("%f ", tab_of_time_openmp[i]);
        }
    }
}