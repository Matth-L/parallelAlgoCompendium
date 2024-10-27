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

void pretty_print_array(int *tab, int n)
{
    printf("[");
    for (int i = 0; i < n; i++)
    {
        printf("%d", tab[i]);
        if (i < n - 1)
        {
            printf(", ");
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

    tri_fusion(U, mid);
    tri_fusion(V, (n - mid));
    fusion(U, mid, V, (n - mid), tab);
    free(U);
    free(V);
}

int main(int argc, char *argv[])
{
    /**********************************************
     * reading the file  + init the array
    ***********************************************/
    FILE *f = fopen("array.txt", "r");
    if (f == NULL)
    {
        perror("Error fopen");
        exit(EXIT_FAILURE);
    }

    int c, array_size, count = 0;
    fscanf(f, "%d", &array_size);
    int *T = malloc(array_size * sizeof(int));

    while(!feof (f))
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
    pretty_print_array(T, 16);
    fflush(stdout);

    start = omp_get_wtime();
    tri_fusion(T, array_size);
    stop = omp_get_wtime();

    printf("After sorting:\n");
    pretty_print_array(T, 16);
    printf("\nTime: %g\n",stop-start);

    fflush(stdout);

    exit(EXIT_SUCCESS);
}
