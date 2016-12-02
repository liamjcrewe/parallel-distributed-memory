#include <stdlib.h>

/**
 * Create a square two dimensional array of doubles of the dimension specified.
 * Uses malloc, should always be followed later in the calling code with
 * freeTwoDDoubleArray.
 *
 * @param  rows      Number of rows in double array to be created
 * @param  cols      Number of columns in double array to be created
 *
 * @return           Pointer to the created two dimensional array
 */
double **createTwoDDoubleArray(const int rows, const int cols)
{
    double **createdRows = (double **)malloc(rows * sizeof(double*));

    for (int row = 0; row < rows; row++) {
        createdRows[row] = (double *)malloc(cols * sizeof(double));
    }

    return createdRows;
}

/**
 * Frees a given two dimensional array of doubles of the dimension specified.
 *
 * @param array     The two dimensional array to free
 * @param rows   The number of rows in the two dimensional array to free
 */
void freeTwoDDoubleArray(double **array, const int rows)
{
    for(int row = 0; row < rows; row++) {
        free(array[row]);
    }

    free(array);
}

int doubleArraysEqual(
    double * const array1,
    double * const array2,
    const int dimension
)
{
    for (int i = 0; i < dimension; i++) {
        if (array1[i] != array2[i]) {
            return 0;
        }
    }

    return 1;
}





//NOT USED
void prefixSumInt(int * const input, int * const output, const int size)
{
    int sum;

    for (int i = 0; i < size; i++) {
        sum += input[i];

        output[i] = sum;
    }
}

//NOT USED
int intArraySum(int * const array, const int start, const int end)
{
    int sum = 0;

    for (int i = start; i < end; i++) {
        sum += array[i];
    }

    return sum;
}
