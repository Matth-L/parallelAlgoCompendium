/*******************************************************************************
 * @file d2p.c
 * @brief Implementation of parallel merge sort using pthread
 * 
 ******************************************************************************/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>
#include <omp.h>
#include <semaphore.h>

#define INSERTION_SORT_THRESHOLD 10000

sem_t max_depth;

// Data structure to pass to the thread function
typedef struct Thread_data
{
    int n;
    int *tab;
} data_t;

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
 * @brief Sorts an array of integers using insertion sort
 * @param t The data containing the array to sort
 */
void tri_insertion(data_t t)
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

/**
 * @brief Merges two sorted arrays into one sorted array
 * @param u The first sorted array
 * @param v The second sorted array
 * @param T The resulting merged array
 */
void fusion(data_t u, data_t v, int *T)
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

struct two_data
{
    data_t *to_copy;
    data_t *to_paste;
};

/**
 * @brief Copies the first half of an array into another array
 * @param arg The data containing the array to copy and the array to paste into
 */
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

/**
 * @brief Sorts an array of integers using parallel merge sort with pthread
 * @param arg The data containing the array to sort and its size
 */
void *tri_fusion(void *arg)
{
    data_t *t = (data_t *)arg;
    int value_sem;
    if (t->n < 2)
        return NULL;
    else if (t->n > log2floor(sem_getvalue(&max_depth, &value_sem))) // <= INSERTION_SORT_THRESHOLD)
    {
        tri_insertion(*t);
        return NULL;
    }

    int mid = t->n / 2;

    data_t u = {mid, malloc((mid + 1) * sizeof(int))};
    data_t v = {t->n - mid, malloc((t->n - mid + 1) * sizeof(int))};

    if (u.tab == NULL || v.tab == NULL)
    {
        perror("malloc : u.tab or v.tab error");
        exit(EXIT_FAILURE);
    }

    // Parallel copy of the array
    struct two_data u_data = {t, &u};

    pthread_t copy_u;

    // Thread to copy the first half of u
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

    pthread_t child;

    // Create a new thread to sort the second half of u
    if (pthread_create(&child, NULL, tri_fusion, &u) != 0)
    {
        perror("pthread_create error");
        exit(EXIT_FAILURE);
    }
    // Master thread sorts the second half of v
    tri_fusion(&v);
    sem_post(&max_depth);
    pthread_join(child, NULL);
    fusion(u, v, t->tab);
    return NULL;
}

/**
 * @brief Fills an array with random integers
 * @param tab The array to fill
 * @param n The size of the array
 */
void fill_array(int *tab, int n)
{
    srand(time(NULL));
    for (int i = 0; i < n; i++)
    {
        tab[i] = rand() % 100;
    }
}

int main(int argc, char *argv[])
{
    // Check if the number of arguments is correct
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <size_of_array>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Initialize the semaphore
    sem_init(&max_depth, 0, 0);
    omp_set_num_threads(omp_get_max_threads());

    int array_size = atoi(argv[1]);
    int *T = malloc(array_size * sizeof(int));
    if (T == NULL)
    {
        perror("malloc : T error  ");
        exit(EXIT_FAILURE);
    }

    data_t init_data = {array_size, T};

    double start = omp_get_wtime();
    tri_fusion(&init_data);
    double stop = omp_get_wtime();

    printf("\033[0;32m\nPthread: ");
    printf("\033[0;32m\nTime: %g s\033[0m\n", stop - start);

    free(init_data.tab);

    return 0;
}
