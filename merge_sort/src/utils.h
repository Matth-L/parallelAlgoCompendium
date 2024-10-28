/*******************************************************************************
 * @file utils.h
 * @brief Utility functions for array printing and mathematical operations
 ******************************************************************************/
#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/**
 * @brief Pretty prints an integer array with custom formatting
 *
 * @param tab Pointer to the array
 * @param n Size of the array
 */
void pretty_print_array(int *tab, int n);

/**
 * @brief Computes the floor of the base-2 logarithm of an integer
 *
 * @param n The integer to compute the log2 floor
 * @return int Floor value of the base-2 logarithm of n
 */
int log2floor(int n);

#endif // UTILS_H
