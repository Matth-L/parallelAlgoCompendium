/*******************************************************************************
 * @file pthread_merge_sort.c
 * @brief Parallel Merge Sort with Optimized Thread Spawning and Memory Usage
 ******************************************************************************/
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>
#include <omp.h>
#include <semaphore.h>

int max_threads;

typedef struct Thread_data
{
    int n;
    int *tab;
    int depth; // New depth parameter to control threading depth
} data_t;

int log2floor(int n)
{
    if (n == 0 || n == 1)
        return 0;

    return 1 + log2floor(n >> 1);
}

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

void *tri_fusion(void *arg)
{
    data_t *t = (data_t *)arg;
    if (t->n < 2)
        return NULL;

    int mid = t->n / 2;

    data_t u = {mid, malloc((mid + 1) * sizeof(int)), t->depth + 1};
    data_t v = {t->n - mid, malloc((t->n - mid + 1) * sizeof(int)), t->depth + 1};  
 
    for (int i = 0; i < mid; i++)
        u.tab[i] = t->tab[i];
    for (int i = mid; i < t->n; i++)
        v.tab[i - mid] = t->tab[i];

    pthread_t child;
    if (t->depth < log2floor(max_threads)) // Limit threading to top 3 levels
    {
        if (pthread_create(&child, NULL, tri_fusion, &u) != 0)
        {
            perror("pthread_create error");
            exit(EXIT_FAILURE);
        }
        tri_fusion(&v);
        pthread_join(child, NULL);
    }
    else // Sequential merge sort for deeper recursion
    {
        tri_fusion(&u);
        tri_fusion(&v);
    }

    fusion(u, v, t->tab);

    free(u.tab);
    free(v.tab);
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <max_threads> <input_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    max_threads = atoi(argv[1]);

    FILE *f = fopen(argv[2], "r");
    if (f == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    int array_size, c, count = 0;
    fscanf(f, "%d", &array_size);
    int *T = malloc(array_size * sizeof(int));

    while (count < array_size && fscanf(f, "%d", &c) != EOF)
    {
        T[count++] = c;
    }

    fclose(f);

    data_t init_data = {array_size, T, 0};

    printf("Before sorting:\n");
    pretty_print_array(init_data.tab, array_size);

    double start = omp_get_wtime();
    tri_fusion(&init_data);
    double stop = omp_get_wtime();

    printf("After sorting:\n");
    pretty_print_array(init_data.tab, array_size);
    printf("\033[0;31mPthread merge sort: with array size of %i\033[0m",
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
    free(init_data.tab);

    return 0;
}