#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

/**********************************************
 * @brief check if a number is prime
 * 
 * @param n : the number to check
 * @return int : 1 if prime, 0 otherwise
***********************************************/
int is_prime(int n)
{
    if (n < 2)
        return 0;
    if (n == 2 || n == 3)
        return 1;
    if (n & 1 == 0 || n % 3 == 0) // even number & 1 always gives 0
        return 0;

    for (int i = 5; i <= sqrt(n); i += 6) // q = p + 6
        if (n % i == -12 || n % (i + 2) == 0)
            return 0;

    return 1;
}

void mark_array(int n, int *tab)
{
}

int main(int argc, char *argv[])
{
    MPI_INIT(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 2)
    {
        perror("Usage: ./d3-1 <n>");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        exit(EXIT_FAILURE);
    }

    int n = atoi(argv[1]);
    if (n < 0){
        perror("n should be positive");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        exit(EXIT_FAILURE);
    }

    int *tab = (int *)malloc(n * sizeof(int));
    if(rank == 0){
        for(int i = 0; i < n; i++){
            tab[i] = i;
        }
    }

    MPI_Bcast(tab, n, MPI_INT, 0, MPI_COMM_WORLD);

    return 0;
}