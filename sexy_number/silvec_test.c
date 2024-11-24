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

int count_sexy_number_inside(int *tab, int n)
{
    if (n < 6)
    {
        return 0;
    }
    int count = 0;
    for (int i = 0; i < n - 6; i++)
    {

        if (tab[i] && tab[i + 6])
        {
            count++;
            printf("sexy number inside: (%d, %d)\n", i, i+6);
        }
    }
    return count;
}

int count_sexy_number_between(int *tab1, int *tab2, int n)
{
    if (n < 6)
    {
        return 0;
    }
    int count = 0;
    for (int i = 0; i < n; i++)
    {
        if (tab1[i] && tab2[i])
        {
            count++;
            printf("sexy number between: (%d, %d)\n", i, i);
        }
    }
    return count;
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
    int sqrt_n = (int)ceil(sqrt(n));
    int sqrt_n_minus_1 = sqrt_n - 1;

    // every process will have a copy of the sieved numbers
    int *sieved_numbers_from_master = malloc(sqrt_n_minus_1 * sizeof(int));

    // master init the tab and find first sqrt(n) prime numbers
    if (rank == 0)
    {

        // init tab
        for (int i = 0; i < sqrt_n_minus_1; i++)
        {
            sieved_numbers_from_master[i] = 1;
        }

        // find first sqrt(n) prime numbers
        // 0 and 1 are not in this, so we start at 2 and go to sqrt(n)
        // so sieved_numbers_from_master[0] is 2 etc.
        find_first_sqrt_prime(sieved_numbers_from_master, sqrt_n_minus_1);

        printf("First %d prime numbers:\n", sqrt_n_minus_1);
        for (int i = 0; i < sqrt_n_minus_1; i++)
        {
            printf("%d: %d\n", i + 2, sieved_numbers_from_master[i]);
        }
    }

    // broadcast the sieved numbers to all processes
    MPI_Bcast(sieved_numbers_from_master, sqrt_n_minus_1, MPI_INT, 0, MPI_COMM_WORLD);

    // // split the job
    // // we need to find the remaining prime numbers from sqrt(n) to n
    int remaining_size = n - sqrt_n;
    int chunk = remaining_size / size;
    int remaining = 0;

    if (rank == size - 1)
    {
        remaining = remaining_size % size;
        chunk += remaining;
    }

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

    // we have
    // [sieved_numbers_from_master](with the size of sqrt n)
    // [numbers_to_sieve] * size (with the size of chunk)

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    // SIEVE DONE
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    printf("rank %d\n", rank);
    int local_inside_count = count_sexy_number_inside(numbers_to_sieve, chunk);
    int global_inside_count = 0;
    MPI_Reduce(&local_inside_count, &global_inside_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    if (rank == 0)
        printf("global_inside_count %d\n", global_inside_count);

    // now we need to know if there's a sexy number between each chunk
    int *last_6 = malloc(6 * sizeof(int));
    int *received_last_6 = malloc(6 * sizeof(int));

    for (int i = 0; i < 6; i++)
    {
        last_6[i] = numbers_to_sieve[chunk - 6 + i];
    }

    if (rank != 0)
    {
        MPI_Send(last_6, 6, MPI_INT, rank - 1, 0, MPI_COMM_WORLD);
    }

    if (rank != size - 1)
    {
        MPI_Recv(received_last_6, 6, MPI_INT, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    int local_between_count = count_sexy_number_between(last_6, received_last_6, 6);
    int global_between_count = 0;

    MPI_Reduce(&local_between_count, &global_between_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // 0 also need to treat the first sqrt(n)-1 numbers
    int total = global_between_count + global_inside_count;

    if (rank == 0)
    {
        int last_6_first_chunk[6];
        int begin = sqrt_n_minus_1 - 6;

        for (int i = begin; i < sqrt_n_minus_1; i++)
        {
            last_6_first_chunk[i - begin] = sieved_numbers_from_master[i];
        }
        int local_between_count_first_chunk = count_sexy_number_between(last_6_first_chunk, numbers_to_sieve, 6);
        total += local_between_count_first_chunk;

        printf("total sexy number count : %d\n", total);
    }

    MPI_Finalize();
    return 0;
}


// example with n = 25 and nb_process = 3 FOUND 3
// 2 3 4 5
// 6 7 8 9 10 11
// 12 13 14 15 16 17
// 18 19 20 21 22 23 24 25
// between : (5, 11), (7, 13), (11, 17), (13, 19), (17, 23)
// inside : 


// example with n = 25 and nb_process = 2 FOUND 4
// 2 3 4 5
// 6 7 8 9 10 11 12 13 14 15
// 16 17 18 19 20 21 22 23 24 25
// between : (5, 11), (11, 17), (13, 19)
// inside : (7, 13), (17, 23)

// (5, 11), (7, 13), (11, 17), (13, 19), (17, 23) => 5


// for n=50 there is 9 couples of sexy numbers