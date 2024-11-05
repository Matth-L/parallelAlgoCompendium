/**********************************************
 * @file find_n.c
 * @brief The goal of this program is to find the smallest n such
 * that the parallel version of the merge sort is faster than the
 * sequential version.
 ***********************************************/

#include <omp.h> // for omp_get_wtime
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>
#include <semaphore.h>

/**********************************************
 * @brief Prints the first 100 and last 100 elements of an array
 * if the array is larger than 1000 elements
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
    else // n > 1000
    {
        for (int i = 0; i < 100; i++)
        {
            printf("%d, ", tab[i]);
        }
        printf(" ... ");
        for (int i = n - 100; i < n; i++)
        {
            printf(", %d", tab[i]);
        }
    }
    printf("]\n");
}

void fusion_sequential(int *U, int n, int *V, int m, int *T)
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

void tri_fusion_sequential(int *tab, int n)
{
    if (n < 2)
        return;

    /**********************************************
     *  Split the array into two parts
     ***********************************************/
    int mid = n / 2;
    int *U = malloc((mid + 1) * sizeof(int));
    int *V = malloc((n - mid + 1) * sizeof(int));
    if (U == NULL || V == NULL)
    {
        perror("malloc : U or V error");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < mid; i++)
    {
        U[i] = tab[i];
    }
    for (int i = 0; i < n - mid; i++)
    {
        V[i] = tab[i + mid];
    }
    /**********************************************
     * Sort the two parts + merge them
     ***********************************************/
    tri_fusion_sequential(U, mid);
    tri_fusion_sequential(V, (n - mid));
    fusion_sequential(U, mid, V, (n - mid), tab);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// PTHREAD
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

sem_t max_depth; // Helps finding the maximum depth of for each thread

/**********************************************
 * @brief Pthread requires a struct to pass multiple arguments to a thread
 * @arg n The size of the array
 * @arg tab The array to sort
 ***********************************************/
typedef struct Thread_data
{
    int n;
    int *tab;
} data_t;

/**********************************************
 * @brief Again, we need to pass multiple arguments to a thread
 * The goal is to do a parallel copy of U and V
 *
 * @arg to_copy The array to copy
 * @arg to_paste The array to paste into
 ***********************************************/
struct two_data
{
    data_t *to_copy;
    data_t *to_paste;
};

void tri_insertion_pth(data_t t)
{
    int n = t.n;
    for (int i = 1; i < n; i++)
    {
        int x = t.tab[i];
        int j = i;
        while (j > 0 && t.tab[j - 1] > x)
        {
            t.tab[j] = t.tab[j - 1];
            j--;
        }
        t.tab[j] = x;
    }
}

int log2floor(int n)
{
    if (n == 0 || n == 1)
        return 0;

    return 1 + log2floor(n >> 1);
}

void *copy_array(void *arg)
{
    struct two_data *data = (struct two_data *)arg;
    data_t *to_copy = data->to_copy;
    data_t *to_paste = data->to_paste;
    for (int i = 0; i < (to_copy->n) / 2; i++)
    {
        to_paste->tab[i] = to_copy->tab[i];
    }
    return NULL;
}

void fusion_pth(data_t u, data_t v, int *T)
{
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

void *tri_fusion_pth(void *arg)
{
    int value_sem; // Value of the semaphore max_depth
    data_t *t = (data_t *)arg;

    /**********************************************
     * Base case + Threshold case
     ***********************************************/
    if (t->n < 2)
    {
        return NULL;
    }
    else if (t->n > log2floor(sem_getvalue(&max_depth, &value_sem)))
    // the insertion threshold
    {
        tri_insertion_pth(*t);
        return NULL;
    }

    /**********************************************
     * Starting recursion
     ***********************************************/

    /**********************************************
     * Initialization of parallel splitting
     ***********************************************/
    int mid = t->n / 2;

    data_t u = {mid, malloc((mid + 1) * sizeof(int))};
    data_t v = {t->n - mid, malloc((t->n - mid + 1) * sizeof(int))};
    if (u.tab == NULL || v.tab == NULL)
    {
        perror("malloc : u.tab or v.tab error");
        exit(EXIT_FAILURE);
    }

    struct two_data u_data = {t, &u};
    pthread_t copy_u; // Thread to copy the first half of u

    /**********************************************
     * Parallel splitting
     ***********************************************/

    // slave thread copies the first half of u
    if (pthread_create(&copy_u, NULL, copy_array, &u_data) != 0)
    {
        perror("pthread_create error");
        exit(EXIT_FAILURE);
    }
    // Master thread copies the second half of v
    for (int i = 0; i < mid; i++)
    {
        v.tab[i] = t->tab[i + mid];
    }
    pthread_join(copy_u, NULL);

    /**********************************************
     * Recursive sorting
     ***********************************************/
    pthread_t child;

    // slave thread sorts the first half of u
    if (pthread_create(&child, NULL, tri_fusion_pth, &u) != 0)
    {
        perror("pthread_create error");
        exit(EXIT_FAILURE);
    }

    // Master thread sorts the second half of v
    tri_fusion_pth(&v);

    /**********************************************
     * Merging
     ***********************************************/

    sem_post(&max_depth); // Incrementing the depth
    pthread_join(child, NULL);
    fusion_pth(u, v, t->tab);
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// OPENMP
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define INSERTION_SORT_THRESHOLD 10000

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

void tri_fusion_omp(int *tab, int n)
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
            tri_fusion_omp(U, mid);
            tri_fusion_omp(V, n - mid);
        }
    }
    // implicit barrier
    fusion_sequential(U, mid, V, n - mid, tab);

    free(U);
    free(V);
}

int main(int argc, char *argv[])
{
    if (argc != 1)
    {
        printf("Usage: %s\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int n = 1;
    double sequential_time = 0;
    double parallel_time = 0;

    printf("\nNumber of threads: %d\n", omp_get_max_threads());

    double start, end;

    printf("\nTest for Pthread \n");
    do
    {
        n = n * 2;
        int *T = malloc(n * sizeof(int));
        printf("Size of the array: %d\n", n);

        // sequential
        start = omp_get_wtime();
        tri_fusion_sequential(T, n);
        end = omp_get_wtime();
        sequential_time = end - start;

        printf("Sequential time: %g s\n", sequential_time);

        // pthread
        data_t init_data = {n, T};
        sem_init(&max_depth, 0, 0);
        start = omp_get_wtime();
        tri_fusion_pth(&init_data);
        end = omp_get_wtime();
        parallel_time = end - start;

        printf("Pthread time: %g s\n", parallel_time);
        free(T);
    } while (sequential_time < parallel_time);

    printf("\nTest for OpenMP\n");

    // re-init
    sequential_time = 0;
    parallel_time = 0;
    n = 1;
    do
    {
        n = n * 2;
        int *T = malloc(n * sizeof(int));
        printf("Size of the array: %d\n", n);

        // sequential
        start = omp_get_wtime();
        tri_fusion_sequential(T, n);
        end = omp_get_wtime();
        sequential_time = end - start;

        printf("Sequential time: %g s\n", sequential_time);

        // omp
        start = omp_get_wtime();
        tri_fusion_omp(T, n);
        end = omp_get_wtime();
        parallel_time = end - start;

        printf("Omp time: %g s\n", parallel_time);
        free(T);
    } while (sequential_time < parallel_time);

    exit(EXIT_SUCCESS);
}
