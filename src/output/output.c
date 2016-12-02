#include <stdio.h>

/**
 * Write a two dimensional array of doubles to a given file
 *
 * @param f         File handle to write to
 * @param array     Two dimensional array of doubles to write to file
 * @param dimension Dimension of array
 */
void write2dDoubleArray(
    FILE * const f,
    double ** const array,
    const int dimension
)
{
    for (int row = 0; row < dimension; ++row) {
        for (int col = 0; col < dimension; ++col) {
            fprintf(f, "%10f ", array[row][col]);
        }
        fputs("\n", f);
    }
}
