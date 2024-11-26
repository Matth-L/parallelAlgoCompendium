#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <mpi.h>
#include <omp.h> // for omp_get_wtime

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

/**********************************************
 * @brief Find the first sqrt(n) prime numbers
 *
 * @param tab
 * @param n
 ***********************************************/
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

/**********************************************
 * @brief Count the number of sexy numbers inside a tab
 * No communication needed
 *
 * @param tab the tab to check
 * @param n the size of the tab
 * @return int the number of sexy numbers
 ***********************************************/
int count_sexy_number_inside(int *tab, int n, int rank)
{
    int count = 0;
    for (int i = 0; i < n - 6; i++)
    {

        if (tab[i] && tab[i + 6])
        {
            count++;
            printf("rank %d, sexy number inside %d and %d\n", rank, i, i + 6);
        }
    }
    return count;
}

/**********************************************
 * @brief Count the number of sexy numbers between two tabs
 * The tab to process is already given
 *
 * @param tab1
 * @param tab2
 * @param n
 * @param rank
 * @return int
 ***********************************************/
int count_sexy_number_between(int *tab1, int *tab2, int n, int rank)
{

    int count = 0;
    for (int i = 0; i < n; i++)
    {
        if (tab1[i] && tab2[i])
        {
            count++;
            printf("rank %d, sexy number between %d and %d\n", rank, i, i + 6);
        }
    }
    return count;
}

/**
 * @brief If there are too many threads for the size of the tab, we will reduce
 * the number of threads and adjust the size of the chunk.
 *
 * @param nb_process
 * @param size_of_chunk
 * @param remaining_size
 * @param remainder
 */
void resizer(int *nb_process, int *size_of_chunk, int remaining_size,
             int *remainder)
{
    int new_nb_process = *nb_process;
    int new_chunk = remaining_size / new_nb_process;

    // if the number of threads is too big or
    if (new_chunk < 6)
    {
        new_chunk = (remaining_size / 6);
    }

    new_nb_process = (remaining_size / new_chunk);

    *remainder = remaining_size % new_nb_process;
    *nb_process = new_nb_process;
    *size_of_chunk = new_chunk;
    printf("new nb_process %d, new chunk %d, new remainder %d\n",
           *nb_process, *size_of_chunk, *remainder);
}

int main(int argc, char **argv)
{
    double start_sieve = omp_get_wtime();

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

    // split the job
    // we need to find the remaining prime numbers from sqrt(n) to n s
    int remaining_size = n - sqrt_n;
    int chunk = remaining_size / nb_process;
    int remaining = 0;

    // COMMUNICATOR FOR PROCESS
    // there might be too much threads for the size of the tab
    // we will kill the excess of threads
    if (rank == 0)
    {
        resizer(&nb_process, &chunk, remaining_size, &remaining);
    }

    MPI_Bcast(&nb_process, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&chunk, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&remaining, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // COMMUNICATOR FOR PROCESS TO KILL
    MPI_Comm to_kill;
    if (rank >= nb_process)
    {
        MPI_Comm_split(MPI_COMM_WORLD, 1, rank, &to_kill);
        MPI_Abort(to_kill, 0);
        return 0;
    }

    // the last one will be bigger, with the remainder
    // TODO should change when threads number is too big
    if (rank == nb_process - 1)
    {
        remaining = remaining_size % nb_process;
        chunk += remaining;
    }

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
    }

    // broadcast the sieved numbers to all processes
    MPI_Bcast(first_sqrt, sqrt_n_minus_1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != nb_process - 1)
    {
        remaining = 0;
    }

    // printf("rank %d, nb_process %d, chunk %d, remaining %d\n", rank, nb_process,
    //        chunk, remaining);
    // finding the range
    int *numbers_to_sieve = malloc(chunk * sizeof(int));
    int range_start = sqrt_n + chunk * rank + 1 - remaining * rank;
    int range_end = sqrt_n + chunk * (rank + 1) - remaining * rank;

    // init
    for (int i = 0; i < chunk; i++)
    {
        numbers_to_sieve[i] = 1;
    }

    int step = 0;

    // cross out the numbers
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

    double end_sieve = omp_get_wtime();

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

    double start_counting_couple = omp_get_wtime();

    // finding sexy_numbers inside each chunk
    int local_inside_count = count_sexy_number_inside(numbers_to_sieve, chunk, rank);
    int global_inside_count = 0;
    MPI_Reduce(&local_inside_count, &global_inside_count, 1, MPI_INT, MPI_SUM,
               0, MPI_COMM_WORLD);

    // now we need to know if there's a sexy number between each chunk

    // we use the last 6 of the previous rank to check if there's a sexy number
    // 0 will only send, the last will only recv

    int *received_last_6 = malloc(6 * sizeof(int));

    if (rank != nb_process - 1)
    {
        MPI_Send(&numbers_to_sieve[chunk - 6], 6, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
    }

    int local_between_count = 0;
    int global_between_count = 0;
    int first_prime_number_count = 0;

    if (rank != 0)
    {
        MPI_Recv(received_last_6,
                 6,
                 MPI_INT,
                 rank - 1,
                 0,
                 MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);

        local_between_count = count_sexy_number_between(numbers_to_sieve,
                                                        received_last_6,
                                                        6,
                                                        rank);
    }
    else // 0 counts the first prime number while the other counts between
    {
        if (sqrt_n > 6)
        {
            local_between_count += count_sexy_number_inside(first_sqrt, sqrt_n_minus_1, rank);
            int min_size = MIN(sqrt_n_minus_1, 6);
            local_between_count += count_sexy_number_between(&first_sqrt[sqrt_n_minus_1 - 6], numbers_to_sieve, min_size, rank);
        }
    }

    MPI_Reduce(&local_between_count,
               &global_between_count,
               1,
               MPI_INT,
               MPI_SUM,
               0, MPI_COMM_WORLD);

    // now we need to check with the first sqrt numbers
    if (rank == 0)
    {
        int total = global_between_count + global_inside_count;
        printf("total : %d\n", total);
    }

    double end_counting_couple = omp_get_wtime();

    if (rank == 0)
    {
        printf("Time to sieve: %f\n", end_sieve - start_sieve);
        printf("Time to count: %f\n", end_counting_couple - start_counting_couple);
    }

    free(first_sqrt);
    free(numbers_to_sieve);
    free(received_last_6);
    MPI_Finalize();
    return 0;
}
