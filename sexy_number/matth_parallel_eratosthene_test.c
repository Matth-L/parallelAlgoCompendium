#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <mpi.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

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

int count_sexy_number_inside(int *tab, int n)
{
    int count = 0;
    for (int i = 0; i < n - 6; i++)
    {

        if (tab[i] && tab[i + 6])
        {
            count++;
        }
    }
    return count;
}

int count_sexy_number_between(int *tab1, int *tab2, int n, int rank)
{

    int count = 0;
    for (int i = 0; i < n; i++)
    {
        if (tab1[i] && tab2[i])
        {
            count++;
            printf("rank : %d;  sexy number between: (%d, %d)\n", rank, i, i + 6);
        }
    }
    return count;
}

void resize_size(int *nb_process, int *size_of_chunk, int *remaining_size,
                 int *remainder)
{
    int new_th_num = *nb_process;
    int new_chunk = *remaining_size / new_th_num;

    // if the number of threads is too big or
    if (new_chunk < 6)
    {
        new_chunk = (*remaining_size / 6);
        printf("Reduced the number of threads used\n");
    }

    new_th_num = (*remaining_size / new_chunk);

    *remainder = *remaining_size % new_th_num;
    *nb_process = new_th_num;
    *size_of_chunk = new_chunk;
}

int main(int argc, char **argv)
{

    // MPI init
    MPI_Init(&argc, &argv);
    int rank, nb_process;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nb_process);

    // check args
    if (argc != 2)
    {
        printf("Usage: %s <n>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int n = atoi(argv[1]);
    int sqrt_n = (int)ceil(sqrt(n));
    int sqrt_n_minus_1 = sqrt_n - 1;

    // // split the job
    // // we need to find the remaining prime numbers from sqrt(n) to n s
    int remaining_size = n - sqrt_n;
    int chunk = remaining_size / nb_process;
    int remaining = 0;

    // if (rank == 0)
    // {
    //     resize_size(&nb_process, &chunk, &remaining_size, &remaining);
    //     printf("nb_process : %d\n", nb_process);
    // }

    // MPI_Bcast(&chunk, 1, MPI_INT, 0, MPI_COMM_WORLD);
    // MPI_Bcast(&remaining, 1, MPI_INT, 0, MPI_COMM_WORLD);
    // MPI_Bcast(&nb_process, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // if (rank >= nb_process)
    // {
    //     rank = -1;
    // }

    // every process will have a copy of the sieved numbers
    int *first_sqrt = malloc(sqrt_n_minus_1 * sizeof(int));

    // master init the tab and find first sqrt(n) prime numbers
    if (rank == 0)
    {

        // init tab
        for (int i = 0; i < sqrt_n_minus_1; i++)
        {
            first_sqrt[i] = 1;
        }

        // find first sqrt(n) prime numbers
        // 0 and 1 are not in this, so we start at 2 and go to sqrt(n)
        // so first_sqrt[0] is 2 etc.
        find_first_sqrt_prime(first_sqrt, sqrt_n_minus_1);

        printf("First %d prime numbers:\n", sqrt_n_minus_1);
        for (int i = 0; i < sqrt_n_minus_1; i++)
        {
            printf("%d: %d\n", i + 2, first_sqrt[i]);
        }
    }

    // broadcast the sieved numbers to all processes
    MPI_Bcast(first_sqrt, sqrt_n_minus_1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == nb_process - 1)
    {
        int remaining = remaining_size % nb_process;
        chunk += remaining;
    }

    printf("rank %d, chunk %d\n", rank, chunk);

    // process 0 will treat numbers from sqrt(n) to sqrt(n) + chunk
    // process 1 will treat numbers from sqrt(n) + chunk*rank to sqrt(n) + chunk*(rank+1)
    // process N will treat numbers from sqrt(n) + chunk*(N-1) to n

    int *numbers_to_sieve = malloc(chunk * sizeof(int));
    int range_start = sqrt_n + chunk * rank + 1 - remaining * rank;
    int range_end = sqrt_n + chunk * (rank + 1) - remaining * rank;

    // initialize numbers_to_sieve to 1
    for (int i = 0; i < chunk; i++)
    {
        numbers_to_sieve[i] = 1;
    }

    int step = 0;
    // [sqrt(n), chunk]
    for (int i = 0; i <= sqrt_n_minus_1; i++)
    {
        if (first_sqrt[i])
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
    // first_sqrt = [1, 1, 0, 1, 1 , 1, 0] (2 3 4 5 6 7 8)
    // p0 = (9 10 11 12 13 14 15 16 17 18 19 20 21 22 )
    //      [0 0  1  0  1  0  0  0  1  0  1  0  0  0  ]

    // we have
    // [first_sqrt](with the size of sqrt n)
    // [numbers_to_sieve] * size (with the size of chunk)

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    // SIEVE DONE
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    int local_inside_count = count_sexy_number_inside(numbers_to_sieve, chunk);
    int global_inside_count = 0;
    MPI_Reduce(&local_inside_count, &global_inside_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        printf("total inside: %d\n", global_inside_count);
    }

    // now we need to know if there's a sexy number between each chunk
    int *last_6 = malloc(6 * sizeof(int));
    int *received_last_6 = malloc(6 * sizeof(int));

    for (int i = 0; i < 6; i++)
    {
        last_6[i] = numbers_to_sieve[chunk - 6 + i];
    }

    if (rank != nb_process - 1)
    {
        MPI_Send(last_6, 6, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
    }

    int local_between_count = 0;

    if (rank != 0)
    {
        MPI_Recv(received_last_6, 6, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        local_between_count = count_sexy_number_between(numbers_to_sieve, received_last_6, 6, rank);
    }

    int global_between_count = 0;

    MPI_Reduce(&local_between_count, &global_between_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // now we need to check with the prime_sieve

    if (rank == 0)
    {
        int min_size = MIN(sqrt_n_minus_1, chunk);
        int last_num_first_prime[min_size];
        int last_count = 0;

        for (int i = 0; i < min_size; i++)
        {
            if (first_sqrt[i] && numbers_to_sieve[i + 6 - min_size])
            {
                last_count++;
            }
        }
        int total = global_between_count + global_inside_count + last_count;
        printf("total : %d\n", total);
    }

    MPI_Finalize();
    return 0;
}
