/*******************************************************************************
 * @file d2p.c
 * @brief Implementation of parallel merge sort using pthread
 ******************************************************************************/

#include <omp.h> // for omp_get_wtime
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>
#include <semaphore.h>

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

/**********************************************
 * @brief Computes the floor of the base-2 logarithm of n
 * @param n The integer to compute the logarithm for
 * @return The floor of the base-2 logarithm of n
 ***********************************************/
int log2floor(int n)
{
    if (n == 0 || n == 1)
        return 0;

    return 1 + log2floor(n >> 1);
}

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
 * @brief Sorts array of integers with insertion sort
 * @param t {n, tab}
 ***********************************************/
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

/**********************************************
 * @brief Merges two sorted arrays into one sorted array
 * @param u {n, tab} array 1
 * @param v {n, tab} array 2
 * @param T The resulting merged array
 ***********************************************/
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

/**********************************************
 * @brief Copies the first half of an array into another array
 * @param arg The data containing the array to copy and the array to paste into
 ***********************************************/
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

/**********************************************
 * @brief Sorts an array of integers using parallel merge sort with pthread
 * @param arg The data containing the array to sort and its size
 ***********************************************/
void *tri_fusion(void *arg)
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
        tri_insertion(*t);
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
    if (pthread_create(&child, NULL, tri_fusion, &u) != 0)
    {
        perror("pthread_create error");
        exit(EXIT_FAILURE);
    }

    // Master thread sorts the second half of v
    tri_fusion(&v);

    /**********************************************
     * Merging
     ***********************************************/

    sem_post(&max_depth); // Incrementing the depth
    pthread_join(child, NULL);
    fusion(u, v, t->tab);
    return NULL;
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
    data_t init_data = {array_size, T};

    /**********************************************
     *  Semaphore initialization + Number of threads
     ***********************************************/

    sem_init(&max_depth, 0, 0);
    omp_set_num_threads(omp_get_max_threads());
    printf("\nNumber of threads: %d\n", omp_get_max_threads());

    /**********************************************
     * Sort
     ***********************************************/
    printf("Before sorting:\n");
    pretty_print_array(T, array_size);
    fflush(stdout);

    double start = omp_get_wtime();
    tri_fusion(&init_data);
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
