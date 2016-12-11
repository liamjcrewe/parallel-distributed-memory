/**
 * Create a square two dimensional array of doubles of the dimensions specified.
 *
 * @param  rows      Number of rows in double array to be created
 * @param  cols      Number of columns in double array to be created
 *
 * @return           Pointer to the created two dimensional array
 */
double **createTwoDDoubleArray(const int rows, const int cols);

/**
 * Frees a given two dimensional array of doubles of the dimension specified.
 *
 * @param array     The two dimensional array to free
 * @param dimension The dimension of the two dimensional array to free
 */
void freeTwoDDoubleArray(double **array, const int dimension);
