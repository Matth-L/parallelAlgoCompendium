#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <mpi.h>
#include <omp.h> // for omp_get_wtime

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

/**********************************************
 * @brief Find the first sqrt(n) prime numbers.
 *
 * @param tab the tab to sieve.
 * @param n the size of the tab.
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
 * @brief Count the number of sexy numbers inside a tab.
 * No communication needed.
 *
 * @param tab the tab to check.
 * @param n the size of the tab.
 * @return int the number of sexy numbers.
 ***********************************************/
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

/**********************************************
 * @brief Count the number of sexy numbers between two tabs.
 * The tab to process is already given.
 *
 * @param tab1 the first tab.
 * @param tab2 the second tab.
 * @param n the size of the tab.
 * @param rank the rank of the process.
 * @return int the number of sexy numbers.
 ***********************************************/
int count_sexy_number_between(int *tab1, int *tab2, int n, int rank)
{

    int count = 0;
    for (int i = 0; i < n; i++)
    {
        if (tab1[i] && tab2[i])
        {
            count++;
        }
    }
    return count;
}

/**
 * @brief If there are too many threads for the size of the tab, we will reduce
 * the number of threads and adjust the size of the chunk.
 *
 * @param nb_process the number of process.
 * @param size_of_chunk the size of the chunk.
 * @param remaining_size the remaining size of the tab.
 * @param remainder the remainder of the division.
 */
void resizer(int *nb_process, int *size_of_chunk, int remaining_size,
             int *remainder)
{
    int new_nb_process = *nb_process;
    int new_chunk = remaining_size / new_nb_process;

    // If the number of threads is too big or
    if (new_chunk < 6)
    {
        new_nb_process = (remaining_size / 6);
        new_chunk = 6;
    }

    *remainder = remaining_size % new_nb_process;
    *nb_process = new_nb_process;
    *size_of_chunk = new_chunk;
}

int main(int argc, char **argv)
{
    double start_sieve = omp_get_wtime();

    // MPI initialization.
    MPI_Init(&argc, &argv);
    int rank, nb_process;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nb_process);

    // Check args.
    if (argc != 2)
    {
        printf("Usage: %s <n>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int n = atoi(argv[1]);
    int sqrt_n = (int)ceil(sqrt(n));
    int sqrt_n_minus_1 = sqrt_n - 1;

    // Split the job.
    // We need to find the remaining prime numbers from sqrt(n) to n s.
    int remaining_size = n - sqrt_n;
    int chunk = remaining_size / nb_process;
    int remaining = 0;
    int broadcast_data[3];
    // COMMUNICATOR FOR PROCESS
    // There might be too much threads for the size of the tab.
    // We will kill the excess of threads.
    if (rank == 0)
    {
        resizer(&nb_process, &chunk, remaining_size, &remaining);
        broadcast_data[0] = nb_process;
        broadcast_data[1] = chunk;
        broadcast_data[2] = remaining;
    }

    MPI_Bcast(broadcast_data, 3, MPI_INT, 0, MPI_COMM_WORLD);
    nb_process = broadcast_data[0];
    chunk = broadcast_data[1];
    remaining = broadcast_data[2];

    // COMMUNICATOR FOR PROCESS TO KILL
    int color = (rank < nb_process) ? 0 : MPI_UNDEFINED;
    MPI_Comm alive;
    MPI_Comm_split(MPI_COMM_WORLD, color, rank, &alive);

    if (color == MPI_UNDEFINED)
    {
        MPI_Finalize();
        exit(EXIT_SUCCESS);
    }

    // The last one will be bigger, with the remainder.
    if (rank == nb_process - 1)
    {
        remaining = remaining_size % nb_process;
        chunk += remaining;
    }

    // Every process will have a copy of the sieved numbers.
    int *first_sqrt = malloc(sqrt_n_minus_1 * sizeof(int));

    if (first_sqrt == NULL)
    {
        printf("Malloc failed\n");
        exit(EXIT_FAILURE);
    }

    // Master init the tab and find first sqrt(n) prime numbers.
    if (rank == 0)
    {
        // init tab
        for (int i = 0; i < sqrt_n_minus_1; i++)
        {
            first_sqrt[i] = 1;
        }

        // Find first sqrt(n) prime numbers.
        // 0 and 1 are not in this, so we start at 2 and go to sqrt(n).
        // So first_sqrt[0] is 2 etc.
        find_first_sqrt_prime(first_sqrt, sqrt_n_minus_1);
    }

    // Broadcast the sieved numbers to all processes.
    MPI_Bcast(first_sqrt, sqrt_n_minus_1, MPI_INT, 0, alive);

    if (rank != nb_process - 1)
    {
        remaining = 0;
    }

    // Finding the range.
    int *numbers_to_sieve = malloc(chunk * sizeof(int));
    if (numbers_to_sieve == NULL)
    {
        printf("Malloc failed\n");
        exit(EXIT_FAILURE);
    }

    int range_start = sqrt_n + chunk * rank + 1 - remaining * rank;
    int range_end = sqrt_n + chunk * (rank + 1) - remaining * rank;

    // Initialization
    for (int i = 0; i < chunk; i++)
    {
        numbers_to_sieve[i] = 1;
    }
    int step = 0;

    // cross out the numbers
    // [sqrt(n), chunk]
    for (int i = 0; i < sqrt_n_minus_1; i++)
    {
        if (first_sqrt[i])
        {
            step = i + 2;
            int first_multiple =
                MAX(range_start + (step - range_start % step) % step,
                    step * step);
            for (int j = first_multiple; j <= range_end; j += step)
            {
                numbers_to_sieve[j - range_start] = 0;
            }
        }
    }

    double end_sieve = omp_get_wtime();

    // From this point, they all have an array of size chunk with
    // 1 if the number is prime, 0 otherwise.
    // Example : n = 64 , size = 4, chunk = 14
    // first_sqrt = [1, 1, 0, 1, 1 , 1, 0] (2 3 4 5 6 7 8)
    // p0 = (9 10 11 12 13 14 15 16 17 18 19 20 21 22 )
    //      [0 0  1  0  1  0  0  0  1  0  1  0  0  0  ]

    // We have
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

    // Finding sexy_numbers inside each chunk.
    int local_inside_count = count_sexy_number_inside(numbers_to_sieve,
                                                      chunk);
    // Print the first 10 number of numbers_to_sieve.

    int global_inside_count = 0;

    MPI_Reduce(&local_inside_count, &global_inside_count, 1, MPI_INT, MPI_SUM,
               0, alive);

    // Now we need to know if there's a sexy number between each chunk.

    // We use the last 6 of the previous rank to check if there's a sexy number.
    // 0 will only send, the last will only recv.

    int *received_last_6 = malloc(6 * sizeof(int));
    if (received_last_6 == NULL)
    {
        printf("Malloc failed\n");
        exit(EXIT_FAILURE);
    }

    if (rank != nb_process - 1)
    {
        MPI_Send(&numbers_to_sieve[chunk - 6], 6, MPI_INT, rank + 1, 0, alive);
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
                 alive,
                 MPI_STATUS_IGNORE);

        local_between_count += count_sexy_number_between(numbers_to_sieve,
                                                         received_last_6,
                                                         6,
                                                         rank);
    }
    else // 0 counts the first prime number while the other counts between.
    {
        if (sqrt_n > 6)
        {
            local_between_count +=
                count_sexy_number_inside(first_sqrt,
                                         sqrt_n_minus_1);
        }
        int min_size = MIN(sqrt_n_minus_1, 6);
        local_between_count +=
            count_sexy_number_between(&first_sqrt[sqrt_n_minus_1 - min_size],
                                      numbers_to_sieve,
                                      min_size,
                                      rank);
    }

    MPI_Reduce(&local_between_count,
               &global_between_count,
               1,
               MPI_INT,
               MPI_SUM,
               0, alive);

    // Now we need to check with the first sqrt numbers.
    if (rank == 0)
    {
        int total = global_between_count + global_inside_count;
        double end_counting_couple = omp_get_wtime();
        printf("Sexy number count : %d\n", total);
        printf("Number of process used : %d\n", nb_process);
        printf("Time to sieve: %f\n",
               end_sieve - start_sieve);
        printf("Time to count: %f\n",
               end_counting_couple - start_counting_couple);
        printf("Total time: %f\n",
               end_counting_couple - start_sieve);
    }

    free(first_sqrt);
    free(numbers_to_sieve);
    free(received_last_6);
    MPI_Finalize();
    return 0;
}
