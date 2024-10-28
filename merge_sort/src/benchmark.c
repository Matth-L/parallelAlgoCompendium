#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>
#include <omp.h>
#include <semaphore.h>
#include "pthread_merge_sort.h"
#include "openMP_merge_sort.h"
#include "sequential_merge_sort.h"

int main(int argc, char *argv[])
{
    /**********************************************
     * reading the file  + init the array
     ***********************************************/
    FILE *f = fopen(argv[1], "r");
    printf("File: %s\n", argv[1]);  

    if (f == NULL)
    {
        perror("Error fopen");
        exit(EXIT_FAILURE);
    }

    int c, array_size, count = 0;
    fscanf(f, "%d", &array_size);
    int *T = malloc(array_size * sizeof(int));

    while (!feof(f))
    {
        fscanf(f, "%d", &c);
        T[count] = c;
        count++;
    }

    fclose(f);

    int n_try;
    int tab_of_time_sequential[n_try];
    int tab_of_time_phtread[n_try];
    int tab_of_time_openmp[n_try];

    /**********************************************
     * BENCHMARK SEQUENTIAL
     ***********************************************/

    for(int i = 0; i < n_try; i++){
        double start = omp_get_wtime();
        tri_fusion_sequential(T, array_size);
        double stop = omp_get_wtime();
        tab_of_time_phtread[i] = stop - start;
    }
    /**********************************************
     * BENCHMARK PTHREAD
     ***********************************************/
    data_t init_data = {array_size, T, 0};

    for(int i = 0; i < n_try; i++){
        double start = omp_get_wtime();
        tri_fusion_pthread(&init_data);
        double stop = omp_get_wtime();
        tab_of_time_phtread[i] = stop - start;
    }

    /**********************************************
     * BENCHMARK OPENMP
     ***********************************************/

    for(int i = 0; i < n_try; i++){
        double start = omp_get_wtime();
        tri_fusion_openmp(T, array_size);
        double stop = omp_get_wtime();
        tab_of_time_openmp[i] = stop - start;
    }

    /**********************************************
     * average of tab
     ***********************************************/

    double average_sequential = 0;
    double average_pthread = 0;
    double average_openmp = 0;

    for (int i = 0; i < n_try; i++){
        average_sequential += tab_of_time_sequential[i];
        average_pthread += tab_of_time_phtread[i];
        average_openmp += tab_of_time_openmp[i];
    }

    average_sequential /= n_try;
    average_pthread /= n_try;
    average_openmp /= n_try;

    printf("Average time for sequential: %g s\n", average_sequential);
    printf("Average time for pthread: %g s\n", average_pthread);
    printf("Average time for openMP: %g s\n", average_openmp);

}