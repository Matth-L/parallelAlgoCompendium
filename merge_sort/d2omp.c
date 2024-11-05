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
#include <unistd.h>

#define INSERTION_SORT_THRESHOLD 10000

/**********************************************
 * @brief Prints an array of integers
 * @param tab The array to print
 * @param n The size of the array
 ***********************************************/
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

/**********************************************
 * @brief Merges two sorted arrays into one sorted array
 * @param U The first sorted array
 * @param n The size of the first array
 * @param V The second sorted array
 * @param m The size of the second array
 * @param T The resulting merged array
 ***********************************************/
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

/**********************************************
 * @brief Sorts an array of integers using insertion sort
 * @param tab The array to sort
 * @param n The size of the array
 ***********************************************/
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

/**********************************************
 * @brief Sorts an array of integers using parallel merge sort with OpenMP
 * @param tab The array to sort
 * @param n The size of the array
 ***********************************************/
void tri_fusion(int *tab, int n)
{

    /**********************************************
     * Base case + Threshold case
     ***********************************************/
    if (n < 2)
        return;
    else if (n <= INSERTION_SORT_THRESHOLD)
    {
        tri_insertion(tab, n);
        return;
    }

    /**********************************************
     * Starting recursion
     ***********************************************/

    /**********************************************
     * Initialization of parallel splitting
     ***********************************************/
    int mid = n / 2;
    int *U = malloc((mid + 1) * sizeof(int));
    int *V = malloc((n - mid + 1) * sizeof(int));

    if (U == NULL || V == NULL)
    {
        perror("malloc : U or V error");
        exit(EXIT_FAILURE);
    }

// Every thread has access to this part of the code
#pragma omp parallel sections
    {
// Master gets one, slave gets the other
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
    /**********************************************
     * Recursive sorting
     ***********************************************/

#pragma omp parallel
    {
#pragma omp single
        {
// Master thread sorts U, slave V
#pragma omp task
            tri_fusion(U, mid);
            tri_fusion(V, n - mid);
        }
    }
    // implicit barrier
    fusion(U, mid, V, n - mid, tab);

    free(U);
    free(V);
}
/**********************************************
 * @brief Read the given input file and store the values in the array T
 *
 * @param filename
 * @param array_size
 * @param T the array to store the values
 ***********************************************/
void read_input_file(char *filename, int *array_size, int **T)
{
    FILE *f = fopen(filename, "r");
    if (f == NULL)
    {
        perror("Error fopen");
        exit(EXIT_FAILURE);
    }

    int c, count = 0;
    fscanf(f, "%d", array_size);
    *T = malloc(*array_size * sizeof(int));
    if (*T == NULL)
    {
        perror("malloc : T error for argc == 3");
        exit(EXIT_FAILURE);
    }

    while (!feof(f))
    {
        fscanf(f, "%d", &c);
        (*T)[count] = c;
        count++;
    }

    fclose(f);
}

/**********************************************
 * @brief Write the sorted array to the given output file
 *
 * @param filename
 * @param array_size
 * @param T, the sorted array
 ***********************************************/
void write_output_file(char *filename, int array_size, int *T)
{
    FILE *f_out = fopen(filename, "w");
    if (f_out == NULL)
    {
        perror("Error fopen");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < array_size; i++)
    {
        fprintf(f_out, "%d ", T[i]);
    }

    fclose(f_out);
}

/**
 * @brief Entry point of the program
 * @param argc The number of command-line arguments
 * @param argv The command-line arguments
 * @return The exit code of the program
 */
int main(int argc, char *argv[])
{
    /**********************************************
     * Initialization
     ***********************************************/

    // argc = 2 : ./d2p <size_of_array>
    // argc = 3 : ./d2p <input_file> <output_file>
    if (argc != 2 && argc != 3)
    {
        fprintf(stderr, "Usage: %s <size_of_array>\n", argv[0]);
        fprintf(stderr, "OR\n");
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int *T;
    int array_size;

    if (argc == 2)
    {
        // ./d2p <size_of_array>
        array_size = atoi(argv[1]);
        T = malloc(array_size * sizeof(int));
        if (T == NULL)
        {
            perror("malloc : T error, for argc == 2");
            exit(EXIT_FAILURE);
        }
        // we will sort the memory allocated
    }
    else // argc == 3
    {
        // ./d2p <input_file> <output_file>
        if (access(argv[1], F_OK) == -1 || access(argv[2], F_OK) == -1)
        {
            fprintf(stderr, "One of the given file does not exist\n");
            exit(EXIT_FAILURE);
        }
        read_input_file(argv[1], &array_size, &T);
    }

    // Run with max threads
    omp_set_num_threads(omp_get_max_threads());
    printf("\nNumber of threads: %d\n", omp_get_max_threads());

    /**********************************************
     * Sort
     ***********************************************/
    printf("Before sorting:\n");
    pretty_print_array(T, array_size);
    fflush(stdout);

    double start = omp_get_wtime();
    tri_fusion(T, array_size);
    double stop = omp_get_wtime();

    /**********************************************
     * Print after sorting
     ***********************************************/
    printf("After sorting:\n");
    pretty_print_array(T, array_size);
    printf("\033[0;32m\nTime: %g s\n\033[0m", stop - start);
    fflush(stdout);

    if (argc == 3)
    {
        /**********************************************
         * Writing the sorted array to a file
         ***********************************************/
        write_output_file(argv[2], array_size, T);
    }
    free(T);
    exit(EXIT_SUCCESS);
}