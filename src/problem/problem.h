/**
 * Get the dimension of the specified problem
 *
 * @param  problemId The ID of the problem
 *
 * @return           The integer dimension of the specified problem, or
 *                   -1 if no problem exists for the given problemId
 */
int getProblemDimension(const int problemId);

/**
 * Fill the given two dimensional array with the values of the specified problem
 *
 * @param  values    The array to fill
 * @param  problemId The ID of the problem to copy into values
 *
 * @return           0 if success, -1 if error
 */
int fillProblemArray(double ** const values, const int problemId);
