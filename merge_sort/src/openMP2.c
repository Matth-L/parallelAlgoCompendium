/*******************************************************************************
 * @file sequential_merge_sort.c
 * @brief Parallel Merge Sort using OpenMP
 ******************************************************************************/
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

void pretty_print_array(int *tab, int n)
{
    printf("[");
    if (n < 1000)
    {
        for (int i = 0; i < n; i++)
        {
            printf("%d", tab[i]);
            if (i < n - 1)
            {
                printf(", ");
            }
        }
    }
    else
    {
        for (int i = 0; i < 100; i++)
        {
            printf("%d", tab[i]);
            if (i < 99)
            {
                printf(", ");
            }
        }
        printf(", ... , ");
        for (int i = n - 100; i < n; i++)
        {
            printf("%d", tab[i]);
            if (i < n - 1)
            {
                printf(", ");
            }
        }
    }
    printf("]\n");
}

void fusion(int *U, int n, int *V, int m, int *T)
{
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

void tri_fusion(int *tab, int n)
{
    if (n < 2)
    {
        return;
    }

    int mid = n / 2;
    int *U = malloc((mid + 1) * sizeof(int));
    int *V = malloc((n - mid + 1) * sizeof(int));

    for (int i = 0; i < mid; i++)
    {
        U[i] = tab[i];
        V[i] = tab[i+mid];
    }

    #pragma omp taskgroup
    {
        #pragma omp task shared(U) untied if (n >= (1<<14))
        tri_fusion(U, mid);
        #pragma omp task shared(V) untied if (n >= (1<<14))
        tri_fusion(V, (n - mid));
        #pragma omp taskyield
    }

    fusion(U, mid, V, (n - mid), tab);

    free(U);
    free(V);
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    /**********************************************
     * parsing thread count
     ***********************************************/
    int num_threads = atoi(argv[1]);
    omp_set_num_threads(num_threads);

    /**********************************************
     * reading the file  + init the array
     ***********************************************/
    FILE *f = fopen(argv[2], "r");
    printf("File: %s\n", argv[2]);
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

    /**********************************************
     * sorting
     ***********************************************/
    double start = omp_get_wtime();
    #pragma omp parallel
    {
        #pragma omp single
        tri_fusion(T, array_size);
    }
    double stop = omp_get_wtime();

    printf("After sorting:\n");
    pretty_print_array(T, array_size);
    printf("\n\033[0;31mSequential merge sort: with array size of %i\033[0m", array_size);
    printf("\033[0;32m\nTime: %g s\n\033[0m", stop - start);
    fflush(stdout);

    /**********************************************
     * writing the sorted array in a file
     ***********************************************/
    char output_filename[50];
    snprintf(output_filename, sizeof(output_filename), "sorted_array_%d.txt", array_size);

    FILE *f_out = fopen(output_filename, "w");
    if (f_out == NULL)
    {
        perror("Error fopen");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < array_size; i++)
    {
        fprintf(f_out, "%d\n", T[i]);
    }

    fclose(f_out);
    free(T);

    exit(EXIT_SUCCESS);
}