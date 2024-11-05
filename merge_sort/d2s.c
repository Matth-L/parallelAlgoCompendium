/*******************************************************************************
 * @file d2s.c

 * @brief Sequential merge sort, algorithm from the course "Parallel programming
 * on parallel and distributed systems" at the UQAC.
 *
 ******************************************************************************/
#include <omp.h> // for omp_get_wtime
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

/**********************************************
 * @brief Merges two sorted arrays into one sorted array
 * @param U The first sorted array
 * @param n The size of the first array
 * @param V The second sorted array
 * @param m The size of the second array
 * @param T The resulting merged array
 *
 * @code
 * Algorithm :
 * procedure fusion(U[0..n-1],V[0..m-1],T[0..m-1+n-1])
 * i=j=0
 * U[n]=V[m]=∞
 * pour k=0 `a m-1+n-1 faire
 *  si U[i]<V[j] alors
 *      T[k]=U[i++]
 *  sinon
 *      T[k]=V[j++]
 * @endcode
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
 * @brief Sorts an array of integers using recursive merge sort
 * @param tab The array to sort
 * @param n The size of the array
 *
 * @code
 * procedure tri fusion(T[1..n])
 *  si n est petit
 *      adhoc(T[1..n])
 *  sinon
 *      U[1..n/2]=T[1..n/2]
 *      V[1..n/2]=T[1+n/2..n]
 *      tri fusion(U)
 *      tri fusion(V)
 *      fusion(U,V,T)
 * @endcode
 ***********************************************/
void tri_fusion(int *tab, int n)
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
    tri_fusion(U, mid);
    tri_fusion(V, (n - mid));
    fusion(U, mid, V, (n - mid), tab);
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

    // argc = 2 : ./d2s <size_of_array>
    // argc = 3 : ./d2s <input_file> <output_file>
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
        // ./d2s <size_of_array>
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
        if (access(argv[1], F_OK) == -1 || access(argv[2], F_OK) == -1)
        {
            fprintf(stderr, "One of the given file does not exist\n");
            exit(EXIT_FAILURE);
        }
        read_input_file(argv[1], &array_size, &T);
    }

    /**********************************************
     * Print before sorting
     ***********************************************/
    printf("Before sorting:\n");
    pretty_print_array(T, array_size);
    fflush(stdout);

    /**********************************************
     * Sort
     ***********************************************/

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

    /**********************************************
     * Writing the sorted array to a file
     ***********************************************/
    write_output_file(argv[2], array_size, T);
    free(T);

    exit(EXIT_SUCCESS);
}
