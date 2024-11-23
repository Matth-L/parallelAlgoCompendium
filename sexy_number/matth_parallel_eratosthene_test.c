#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <mpi.h>

void find_first_sqrt_prime(int *tab, int n)
{
    for (int i = 2; i <= n; i++)
    {
        if (tab[i - 2])
        {
            for (int j = i * i; j <= n + 1; j += i)
            {
                tab[j - 2] = 0;
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

    int n = atoi(argv[1]);
    int sqrt_n_minus_1 = (int)floor(sqrt(n)) - 1;

    // every process will have a copy of the sieved numbers
    int *sieved_numbers_from_master = malloc(sqrt_n_minus_1 * sizeof(int));

    // master init the tab and find first sqrt(n) prime numbers
    if (rank == 0)
    {

        printf("size of the tab: %d\n", sqrt_n_minus_1);
        // init tab
        for (int i = 0; i < sqrt_n_minus_1; i++)
        {
            sieved_numbers_from_master[i] = 1;
        }

        // find first sqrt(n) prime numbers
        // 0 and 1 are not in this, so we start at 2 and go to sqrt(n)
        // so sieved_numbers_from_master[0] is 2 etc.
        find_first_sqrt_prime(sieved_numbers_from_master, sqrt_n_minus_1);

        // the index should be prime + 2
    }

    // broadcast the sieved numbers to all processes
    MPI_Bcast(sieved_numbers_from_master, sqrt_n_minus_1, MPI_INT, 0, MPI_COMM_WORLD);

    // // split the job
    // // we need to find the remaining prime numbers from sqrt(n) to n
    int remaining_size = n - (int)sqrt(n);
    int chunk = remaining_size / size;

    if (rank == 0)
    {
        printf("remaining size: %d\n", remaining_size);
        printf("chunk size: %d\n", chunk);
    }

    // process 0 will treat numbers from sqrt(n) to sqrt(n) + chunk
    // process 1 will treat numbers from sqrt(n) + chunk*rank to sqrt(n) + chunk*(rank+1)
    // process N will treat numbers from sqrt(n) + chunk*(N-1) to n

    int *numbers_to_sieve = malloc(chunk * sizeof(int));
    int range_start = (int)sqrt(n) + chunk * rank + 1;
    int range_end = (int)sqrt(n) + chunk * (rank + 1);

    // initialize numbers_to_sieve to 1
    for (int i = 0; i < chunk; i++)
    {
        numbers_to_sieve[i] = 1;
    }

    int step = 0;
    // [sqrt(n), chunk]
    for (int i = 0; i <= sqrt_n_minus_1; i++)
    {
        if (sieved_numbers_from_master[i])
        {
            step = i + 2;
            for (int j = range_start; j <= range_end; j++)
            {
                if (j % step == 0)
                {
                    numbers_to_sieve[j - range_start] = 0;
                }
            }
        }
    }

    // from this point, they all have an array of size chunk with
    // 1 if the number is prime, 0 otherwise
    // example : n = 64 , size = 4, chunk = 14
    // sieved_numbers_from_master = [1, 1, 0, 1, 1 , 1, 0] (2 3 4 5 6 7 8)
    // p0 = (9 10 11 12 13 14 15 16 17 18 19 20 21 22 )
    //      [0 0  1  0  1  0  0  0  1  0  1  0  0  0  ]

    // if (rank == 0)
    // {
    //     for (int i = 0; i < chunk; i++)
    //     {
    //         printf("%d ", numbers_to_sieve[i]);
    //     }
    //     printf("\n");
    // }

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    // SIEVE DONE
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    // // gather the results
    // int *results = NULL;
    // if (rank == 0)
    // {
    //     results = malloc(remaining_size * sizeof(int));
    // }

    // MPI_Gather(numbers_to_sieve, chunk, MPI_INT, results, chunk, MPI_INT, 0, MPI_COMM_WORLD);

    // if (rank == 0)
    // {
    //     for (int i = 0; i < sqrt_n_minus_1; i++)
    //     {
    //         if (sieved_numbers_from_master[i])
    //         {
    //             printf("%d\n", i + 2);
    //         }
    //     }
    //     for (int i = 0; i < remaining_size; i++)
    //     {
    //         if (results[i])
    //         {
    //             printf("%d\n", i + (int)sqrt(n) + 1);
    //         }
    //     }
    // }

    MPI_Finalize();

    return 0;
}
