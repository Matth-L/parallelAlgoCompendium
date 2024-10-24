/*******************************************************************************
 * @file pthread_merge_sort.c
 * @author 
 * @brief
 *
 ******************************************************************************/
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>

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

// should stay sequential
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

// should use pthread
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
    {
        U[i] = tab[i];
        V[i] = tab[i + mid];
    }

    tri_fusion(U, n / 2);
    tri_fusion(V, n / 2);
    fusion(U, n / 2, V, n / 2, tab);
    free(U);
    free(V);
}

int main(int argc, char *argv[])
{

    // not generic, just to test
    int array_size = 16;
    int T[array_size];

    // seed for random number generation
    srand(time(NULL));

    for (int i = 0; i < 16; i++)
    {
        T[i] = rand() % 100;
    }

    printf("Before sorting:\n");
    pretty_print_array(T, 16);
    fflush(stdout);

    tri_fusion(T, 16);

    printf("After sorting:\n");
    pretty_print_array(T, 16);
    fflush(stdout);

    return EXIT_SUCCESS;
}
