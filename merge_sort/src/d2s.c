/*******************************************************************************
 * @file sequential_merge_sort.c
 * @author Matthias Lapu,
 * @brief
 *
 ******************************************************************************/
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>


/**
    * @brief pretty print an array
    * @param tab the array to print
    * @param n the size of the array
*/
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

/**
    * @brief merge two sorted arrays
    * @param U the first array
    * @param n the size of the first array
    * @param V the second array
    * @param m the size of the second array
    * @param T the array to store the result
*/
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

/**
    * @brief sort an array using merge sort
    * @param tab the array to sort
    * @param n the size of the array
*/
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

    srand(time(NULL));

    /**********************************************
     * sorting
     ***********************************************/

    double start = omp_get_wtime();
    tri_fusion_sequential(T, array_size);
    double stop = omp_get_wtime();

    printf("After sorting:\n");
    pretty_print_array(T, array_size);
    printf("\n\033[0;31mSequential merge sort: with array size of %i\033[0m",
           array_size);
    printf("\033[0;32m\nTime: %g s\n\033[0m", stop - start);
    fflush(stdout);

    /**********************************************
     * writing the sorted array in a file
     ***********************************************/

    FILE *f_out = fopen(argv[2], "w");
    printf("File: %s\n", argv[2]);
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
    free(T);

    exit(EXIT_SUCCESS);
}
