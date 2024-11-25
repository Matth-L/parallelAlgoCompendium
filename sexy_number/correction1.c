#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

// Fonction pour marquer les nombres non premiers avec le crible d'Ératosthène
void sieve_of_eratosthenes(int N, bool *is_prime) {
    for (int i = 0; i <= N; i++) {
        is_prime[i] = true;
    }
    is_prime[0] = is_prime[1] = false; // 0 et 1 ne sont pas premiers
    
    for (int i = 2; i * i <= N; i++) {
        if (is_prime[i]) {
            for (int j = i * i; j <= N; j += i) {
                is_prime[j] = false;
            }
        }
    }
}

// Fonction principale
int main(int argc, char *argv[]) {
    int rank, size, N;
    int local_count = 0, global_count = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Le processus root lit l'entrée et diffuse N
    if (rank == 0) {
        if (argc != 2) {
            printf("Usage: %s <N>\n", argv[0]);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        N = atoi(argv[1]);
    }
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Allocation et initialisation du crible
    bool *is_prime = (bool *)malloc((N + 1) * sizeof(bool));
    if (rank == 0) {
        sieve_of_eratosthenes(N, is_prime);
    }

    // Diffusion du tableau de primalité
    MPI_Bcast(is_prime, N + 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);

    // Division des tâches
    int block_size = (N + size - 1) / size; // Taille des blocs arrondie vers le haut
    int start = rank * block_size;
    int end = (rank + 1) * block_size - 1;
    if (end > N) end = N;

    // Comptage local des paires sexy
    for (int p = start; p <= end; p++) {
        if (is_prime[p] && p + 6 <= N && is_prime[p + 6]) {
            local_count++;
        }
    }

    // Réduction des résultats
    MPI_Reduce(&local_count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // Résultat final
    if (rank == 0) {
        printf("Nombre de couples sexy <= %d: %d\n", N, global_count);
    }

    free(is_prime);
    MPI_Finalize();
    return 0;
}
