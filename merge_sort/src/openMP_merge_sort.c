/*******************************************************************************
 * @file openMP_merge_sort.c
 * @author
 * @brief
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

int log2floor(int n)
{
    if (n == 0 || n == 1)
        return 0;

    return 1 + log2floor(n >> 1);
}

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

void tri_fusion(int *tab, int n)
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
        V[i] = tab[i+mid];
    }

    if (depth < log2floor(max_threads)) // Limit threading to top 3 levels
    {
#pragma omp parallel sections
        {
#pragma omp section
            tri_fusion(U, mid);
#pragma omp section
            tri_fusion(V, (n - mid));
        }
    }
    else
    {
        tri_fusion(U, mid);
        tri_fusion(V, (n - mid));
    }

    fusion(U, mid, V, (n - mid), tab);
}

int main(int argc, char *argv[])
{
    omp_set_num_threads(atoi(argv[1]));

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

    srand(time(NULL));

    /**********************************************
     * sorting
     ***********************************************/

    printf("Before sorting:\n");
    pretty_print_array(T, array_size);
    fflush(stdout);

    double start = omp_get_wtime();
    tri_fusion(T, array_size);
    double stop = omp_get_wtime();

    printf("After sorting:\n");
    pretty_print_array(T, array_size);
    printf("\n\033[0;31mOpenMP merge sort: with array size of %i\033[0m",
           array_size);
    printf("\033[0;32m\nTime: %g s\n\033[0m", stop - start);
    fflush(stdout);

    /**********************************************
     * writing the sorted array in a file
     ***********************************************/

    char output_filename[50];
    snprintf(output_filename, sizeof(output_filename),
             "sorted_array_%d.txt", array_size);

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