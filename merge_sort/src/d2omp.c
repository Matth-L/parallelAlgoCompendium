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

int max_threads;
int depth = 0;

/**
 * @brief Computes the floor of the base-2 logarithm of n
 * @param n The integer to compute the logarithm for
 * @return The floor of the base-2 logarithm of n
 */
int log2floor(int n)
{
    if (n == 0 || n == 1)
        return 0;

    return 1 + log2floor(n >> 1);
}

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
void fusion_openmp(int *U, int n, int *V, int m, int *T)
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

/**
 * @brief Sorts an array of integers using parallel merge sort with OpenMP
 * @param tab The array to sort
 * @param n The size of the array
 */
void tri_fusion_openmp(int *tab, int n)
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
        return;

    // Split the array into two parts
    int mid = n / 2;
    int *U = malloc((mid + 1) * sizeof(int));
    int *V = malloc((n - mid + 1) * sizeof(int));

    depth++;

    for (int i = 0; i < mid; i++)
    {
        U[i] = tab[i];
        V[i] = tab[i + mid];
    }

    if (depth < log2floor(max_threads)) // Limit threading to top levels
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

/**
 * @brief Entry point of the program
 * @param argc The number of command-line arguments
 * @param argv The command-line arguments
 * @return The exit code of the program
 */
int main(int argc, char *argv[])
{
    omp_set_num_threads(atoi(argv[1]));

    /**********************************************
     * Reading the file and initializing the array
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

    srand(time(NULL));

    /**********************************************
     * Sorting
     ***********************************************/

    printf("Before sorting:\n");
    pretty_print_array(T, array_size);
    fflush(stdout);

    double start = omp_get_wtime();
    tri_fusion_openmp(T, array_size);
    double stop = omp_get_wtime();

    printf("After sorting:\n");
    pretty_print_array(T, array_size);
    printf("\n\033[0;31mOpenMP merge sort: with array size of %i\033[0m",
           array_size);
    printf("\033[0;32m\nTime: %g s\n\033[0m", stop - start);
    fflush(stdout);

    /**********************************************
     * Writing the sorted array to a file
     ***********************************************/

    FILE *f_out = fopen(argv[3], "w");
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