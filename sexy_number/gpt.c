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
            for (int j = i * i; j <= n; j += i)
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

int count_sexy_number_between(int *tab1, int *tab2, int n)
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

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 2)
    {
        if (rank == 0)
            printf("Usage: %s <n>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    int n = atoi(argv[1]);
    if (n < 2)
    {
        if (rank == 0)
            printf("Input must be at least 2.\n");
        MPI_Finalize();
        return 1;
    }

    int sqrt_n = (int)sqrt(n);
    int *sieved_numbers = malloc(sqrt_n * sizeof(int));
    if (!sieved_numbers)
    {
        printf("Memory allocation failed.\n");
        MPI_Finalize();
        return 1;
    }

    // Initialize sieve
    if (rank == 0)
    {
        for (int i = 0; i < sqrt_n; i++)
            sieved_numbers[i] = 1;
        find_first_sqrt_prime(sieved_numbers, sqrt_n);
    }

    MPI_Bcast(sieved_numbers, sqrt_n, MPI_INT, 0, MPI_COMM_WORLD);

    int chunk_size = (n - sqrt_n) / size + 1;
    int range_start = sqrt_n + rank * chunk_size;
    int range_end = (rank == size - 1) ? n : range_start + chunk_size - 1;

    int *local_primes = malloc(chunk_size * sizeof(int));
    for (int i = 0; i < chunk_size; i++)
        local_primes[i] = 1;

    for (int i = 0; i < sqrt_n; i++)
    {
        if (sieved_numbers[i])
        {
            int prime = i + 2;
            for (int j = (range_start + prime - 1) / prime * prime; j <= range_end; j += prime)
            {
                local_primes[j - range_start] = 0;
            }
        }
    }

    int local_count = count_sexy_number_inside(local_primes, range_end - range_start + 1);
    int global_count = 0;
    MPI_Reduce(&local_count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    int *last_6 = malloc(6 * sizeof(int));
    int *recv_6 = malloc(6 * sizeof(int));

    // Extract the last 6 numbers from the current process's chunk
    for (int i = 0; i < 6; i++)
    {
        last_6[i] = (range_end - range_start - 5 + i >= 0) ? local_primes[range_end - range_start - 5 + i] : 0;
    }

    // Send and receive boundary numbers
    if (rank < size - 1)
        MPI_Send(last_6, 6, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
    if (rank > 0)
        MPI_Recv(recv_6, 6, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // Compare boundary numbers to find sexy primes spanning chunks
    int boundary_count = 0;
    if (rank > 0)
        boundary_count = count_sexy_number_between(recv_6, local_primes, 6);

    // Gather boundary counts
    int global_boundary_count = 0;
    MPI_Reduce(&boundary_count, &global_boundary_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // Add the boundary count to the total
    int global_total = 0;
    MPI_Reduce(&local_count, &global_total, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        global_total += global_boundary_count;
        printf("Total sexy prime pairs: %d\n", global_total);
    }

    free(last_6);
    free(recv_6);

    MPI_Finalize();
    return 0;
}
