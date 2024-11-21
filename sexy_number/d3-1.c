#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <mpi.h>

void eratosthene(bool *tab, int array_size)
{
    // init tab
    for (int i = 0; i < array_size; i++)
    {
        tab[i] = true;
    }

    tab[0] = tab[1] = false; // 0 and 1 are not prime numbers

    for (int i = 2; i * i < array_size; i++)
    {
        if (tab[i] == true)
        {
            for (int j = i * i; j < array_size; j += i)
            {
                tab[j] = false;
            }
        }
    }
}

int found(int *tab, int array_size)
{
    int count = 0;
    for (int i = 0; i < array_size; i++)
    {
        for (int j = i; j < array_size; j++)
        {
            if (tab[j + 1] - tab[j] == 6)
            {
                count++;
            }
            else if (tab[j + 1] - tab[j] > 6)
            {
                break;
            }
        }
    }
    return count;
}

// false = not prime
// true = prime
int main(int argc, char **argv)
{

    if (argc != 2)
    {
        printf("Usage: %s <number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    MPI_Init(&argc, &argv);

    int rank, size;

    MPI_Comm_size(MPI_COMM_WORLD, &size); // number of MPI process
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // id
    fflush(stdout);

    if (rank == 0)
    {

        int size_of_tab = 100;
        bool *tab = malloc(size_of_tab * sizeof(bool));

        eratosthene(tab, size_of_tab);

        int prime_count = 0;

        for (int i = 0; i < size_of_tab; i++)
        {
            if (tab[i] == true)
            {
                prime_count++;
            }
        }
        ///////////////////////////////////////////////////////////////////////////

        // cleaning

        int *array_of_prime = malloc(prime_count * sizeof(int));

        for (int i = 0, j = 0; i < size_of_tab; i++)
        {
            if (tab[i] == true)
            {
                array_of_prime[j] = i;
                j++;
            }
        }
        for (int i = 1; i < size; i++)
        {
            MPI_Send(&prime_count, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&array_of_prime, prime_count, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    }
    ///////////////////////////////////////////////////////////////////////////

    int p_count;
    int *tab;

    if (rank != 0)
    {
        MPI_Recv(&p_count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
        MPI_Recv(&tab, p_count, MPI_INT, 0, 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
        int chunk = p_count / size;

        ///////////////////////////////////////////////////////////////////////////
        int sexy_number_found;

        printf("rank %d\n", rank);
        fflush(stdout);
        for (int i = rank * chunk; i < (rank + 1) * chunk; i++)
        {
            found(&tab[rank * chunk], chunk);
        }
    }

    MPI_Finalize();
    return 0;
}
