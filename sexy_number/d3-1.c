#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

void erasthothene(int *tab, int array_size)
{
    for (int to_search = 2; to_search <= (int)sqrt(array_size); to_search++)
    {
        if (tab[to_search] == 0)
        {
            for (int i = to_search * to_search; i < array_size; i += to_search)
            {
                tab[i] = 1;
            }
        }
    }
}
// 0 = prime
// 1 = not prime
int main(int argc, char **argv)
{

    int *tab = calloc(25, sizeof(int));

    erasthothene(tab, 25);

    for (int i = 2; i < 25; i++)
    {
        if (tab[i] == 0)
        {
            printf("%d\n", i);
        }
    }

    free(tab);
}