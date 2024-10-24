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

typedef struct Thread_data
{
    int n;
    int *tab;
} data_t;

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
void fusion(data_t u, data_t v, int *T)
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

// changed the cast to match pthread
void *tri_fusion(void *arg)
{
    data_t *t = (data_t *)arg;

    // procedure tri fusion(T[1..n])
    // si n est petit adhoc(T[1..n])
    // sinon
    // U[1..n/2]=T[1..n/2]
    // V[1..n/2]=T[1+n/2..n]
    // tri fusion(U)
    // tri fusion(V)
    // fusion(U,V,T)

    if (t->n < 2)
        return NULL;

    int mid = t->n / 2;
    data_t u = {mid, malloc(mid * sizeof(int))};
    data_t v = {t->n - mid, malloc((t->n - mid) * sizeof(int))};

    for (int i = 0; i < mid; i++)
        u.tab[i] = t->tab[i];
    for (int i = mid; i < t->n; i++)
        v.tab[i - mid] = t->tab[i];

    // spawn thread
    pthread_t child;
    pthread_create(&child, NULL, (void *(*)(void *))tri_fusion, &u);
    tri_fusion(&v);

    pthread_join(child, NULL);

    fusion(u, v, t->tab);

    free(u.tab);
    free(v.tab);
}

int main(int argc, char *argv[])
{

    // not generic, just to test
    data_t init_data = {16, malloc(init_data.n * sizeof(int))};

    srand(time(NULL));

    for (int i = 0; i < 16; i++)
    {
        init_data.tab[i] = rand() % 100;
    }

    printf("Before sorting:\n");
    pretty_print_array(init_data.tab, 16);
    fflush(stdout);

    tri_fusion(&init_data);

    printf("After sorting:\n");
    pretty_print_array(init_data.tab, 16);
    fflush(stdout);

    free(init_data.tab);

    exit(EXIT_SUCCESS);
}
