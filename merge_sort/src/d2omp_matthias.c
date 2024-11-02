/*******************************************************************************
 * @file d2omp.c
 * @brief Implementation of parallel merge sort using OpenMP
 *
 * This file contains the implementation of a merge sort algorithm using
 * OpenMP to parallelize the sorting. The program reads an array of integers
 * from a file, sorts the array using parallel merge sort, and then writes
 * the sorted array to another file.
 *
 ******************************************************************************/

#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <omp.h>

/**
 * @brief Prints an array of integers
 * @param tab The array to print
 * @param n The size of the array
 */
void pretty_print_array(int *tab, int n)
{
    printf("[");
    if (n <= 1000)
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

/**
 * @brief Merges two sorted arrays into one sorted array
 * @param U The first sorted array
 * @param n The size of the first array
 * @param V The second sorted array
 * @param m The size of the second array
 * @param T The resulting merged array
 */
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

void tri_insertion(int *tab, int n)
{
    for (int i = 1; i < n; i++)
    {
        int x = tab[i];
        int j = i;
        while (j > 0 && tab[j - 1] > x)
        {
            tab[j] = tab[j - 1];
            j--;
        }
        tab[j] = x;
    }
}

void tri_fusion(int *tab, int n)
{
    if (n < 2)
        return;
    else if (n <= 10000)
    {
        tri_insertion(tab, n);
        return;
    }

    // Split the array into two parts
    int mid = n / 2;
    int *U = malloc((mid + 1) * sizeof(int));
    int *V = malloc((n - mid + 1) * sizeof(int));

#pragma omp parallel sections
    {
#pragma omp section
        for (int i = 0; i < mid; i++)
        {
            U[i] = tab[i];
        }

#pragma omp section
        for (int i = 0; i < n - mid; i++)
        {
            V[i] = tab[i + mid];
        }
    }
#pragma omp parallel
    {
#pragma omp single
        {
#pragma omp task
            tri_fusion(U, mid);
            tri_fusion(V, n - mid);
        }
    }

    // Merge the sorted halves
    fusion(U, mid, V, n - mid, tab);

    // Free temporary ys
    free(U);
    free(V);
}

/**
 * @brief Entry point of the program
 * @param argc The number of command-line arguments
 * @param argv The command-line arguments
 * @return The exit code of the program
 */
int main(int argc, char *argv[])
{
    omp_set_num_threads(omp_get_max_threads()); 

    int array_size = 200000000; //   200 000 000

    int *T = malloc(array_size * sizeof(int));
    /**********************************************
     * Sorting
     ***********************************************/

    printf("Before sorting:\n");
    pretty_print_array(T, array_size);
    printf("\n");

    double start = omp_get_wtime();
    tri_fusion(T, array_size);
    double stop = omp_get_wtime();

    printf("After sorting:\n");
    pretty_print_array(T, array_size);
    fflush(stdout);

    printf("Time: %fs\n", stop - start);

    exit(EXIT_SUCCESS);
}