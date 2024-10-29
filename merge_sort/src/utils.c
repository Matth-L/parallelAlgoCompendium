
#include <stdio.h>
/**
 * @brief Computes the floor of the base-2 logarithm of n
 * @param n The integer to compute the logarithm for
 * @return The floor of the base-2 logarithm of n
 */
int log2floor(int n)
{
    if (n == 0 || n == 1)
        return 0;

    return 1 + log2floor(n >> 1);
}

/**
 * @brief Prints an array of integers
 * @param tab The array to print
 * @param n The size of the array
 */  
void pretty_print_array(int *tab, int n)
{
    printf("[");
    if (n <= 1000)
    {
        for (int i = 0; i < n; i++)
        {
            printf("%d", tab[i]);
            if (i < n - 1)
            {
                printf(", ");
            }
        }
    }
    else
    {
        for (int i = 0; i < 100; i++)
        {
            printf("%d", tab[i]);
            if (i < 99)
            {
                printf(", ");
            }
        }
        printf(", ... , ");
        for (int i = n - 100; i < n; i++)
        {
            printf("%d", tab[i]);
            if (i < n - 1)
            {
                printf(", ");
            }
        }
    }
    printf("]\n");
}

