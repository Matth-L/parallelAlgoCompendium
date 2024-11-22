#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <mpi.h>

void find_first_sqrt_prime(int *tab, int n)
{
    for (int i = 2; i * i < n; i++)
    {
        if (tab[i])
        {
            for (int j = i * i; j < n; j += i)
            {
                tab[j] = 0;
            }
        }
    }
}

int main(int argc, char **argv)
{

    // MPI init
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // check args
    if (argc != 2)
    {
        printf("Usage: %s <n>\n", argv[0]);
        return 1;
    }

    // init tab
    int n = atoi(argv[1]);

    // every process will have a copy of the sieved numbers
    int *sieved_numbers_from_master;

    // master init the tab and find first sqrt(n) prime numbers
    if (rank == 0)
    {
        sieved_numbers_from_master = malloc((int)sqrt(n) * sizeof(int));

        sieved_numbers_from_master[0] = sieved_numbers_from_master[1] = 0;

        for (int i = 2; i < n; i++)
        {
            sieved_numbers_from_master[i] = 1;
        }

        // find first sqrt(n) prime numbers
        find_first_sqrt_prime(sieved_numbers_from_master, n);
    }

    // from 0 to sqrt(n) all numbers are prime

    // broadcast the sieved numbers to all processes
    MPI_Bcast(sieved_numbers_from_master, (int)sqrt(n), MPI_INT, 0, MPI_COMM_WORLD);

    // split the job
    // we need to find the remaining prime numbers from sqrt(n) to n
    int remaining_size = n - (int)sqrt(n);
    int chunk = remaining_size / size;

    // process 0 will treat numbers from sqrt(n) to sqrt(n) + chunk
    // process 1 will treat numbers from sqrt(n) + chunk*rank to sqrt(n) + chunk*(rank+1)
    // process N will treat numbers from sqrt(n) + chunk*(N-1) to n

    int *numbers_to_sieve = malloc(chunk * sizeof(int));
    int range_start = (int)sqrt(n) + chunk * rank;
    int range_end = (int)sqrt(n) + chunk * (rank + 1);

    // initialize numbers_to_sieve to 1
    for (int i = 0; i < chunk; i++)
    {
        numbers_to_sieve[i] = 1;
    }

    // [sqrt(n), chunk]
    for (int i = range_start; i < range_end; i++)
    {
        // for each number, we check if it's already in the sieved tab
        // if this number can be divided by a number in the sieved tab, then it's not prime
        // we set it to 0
        for (int j = 2; j * j <= i; j++)
        {
            if (sieved_numbers_from_master[j] && i % j == 0)
            {
                numbers_to_sieve[i - range_start] = 0;
                break;
            }
        }
    }

    // gather the results
    int *results = NULL;
    if (rank == 0)
    {
        results = malloc(remaining_size * sizeof(int));
    }

    MPI_Gather(numbers_to_sieve, chunk, MPI_INT, results, chunk, MPI_INT, 0, MPI_COMM_WORLD);

    // print results
    if (rank == 0)
    {
        for (int i = 0; i < remaining_size; i++)
        {
            if (results[i])
            {
                printf("%d ", (int)sqrt(n) + i);
            }
        }
        printf("\n");
    }

    // clean
    free(numbers_to_sieve);
    if (rank == 0)
    {
        free(sieved_numbers_from_master);
        free(results);
    }

    MPI_Finalize();

    return 0;
}
