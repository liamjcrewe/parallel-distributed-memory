#include <stdlib.h>

/**
 * Create a square two dimensional array of doubles of the dimensions
 * specified. Creates these in a specific way so that all doubles are
 * contiguous in memory.
 *
 * Note: freeTwoDDoubleArray should always be called on the returned array to
 * clean up memory.
 *
 * @param  rows      Number of rows in double array to be created
 * @param  cols      Number of columns in double array to be created
 *
 * @return           Pointer to the created two dimensional array
 */
double **createTwoDDoubleArray(const int rows, const int cols)
{
    double *doubles = (double *)malloc(rows * cols * sizeof(double));

    double **createdRows = (double **)malloc(rows * sizeof(double*));

    for (int row = 0; row < rows; row++) {
        createdRows[row] = &(doubles[row * cols]);
    }

    return createdRows;
}

/**
 * Frees a given two dimensional array of doubles of the dimension specified.
 * Partners the above createTwoDDoubleArray function.
 *
 * @param array     The two dimensional array to free
 * @param rows   The number of rows in the two dimensional array to free
 */
void freeTwoDDoubleArray(double **array, const int rows)
{
    free(&(array[0][0]));

    free(array);
}
