#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <mpi.h>

int main(int argc, char *argv[]) //jsp copilot a auto completé ça
{
    int rank, size;
    int n = 1000000;
    double sum = 0.0;
    double pi = 0.0;
    double x = 0.0;
    double step = 1.0 / (double) n;
    double start_time, end_time;
    int i;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    start_time = MPI_Wtime();

    for (i = rank; i < n; i += size) {
        x = (i + 0.5) * step;
        sum += 4.0 / (1.0 + x * x);
    }

    sum *= step;

    MPI_Reduce(&sum, &pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    end_time = MPI_Wtime();

    if (rank == 0) {
        printf("PI = %.16f\n", pi);
        printf("Time: %f\n", end_time - start_time);
    }

    MPI_Finalize();

    return 0;
}