#include <math.h>

/**
 * Test the given solution was solved to within the given precision.
 *
 * Does one more pass of the solution and returns 0 if any value changes by
 * more than the given precision. If it completes this pass, returns 1 as this
 * means the solution is within precision.
 *
 * @param  solution           The achieved solution
 * @param  solutionDimension  The dimension of the achieved solution
 * @param  precision          The precision the solution should be solved to
 *
 * @return                    1 if solution within precision, 0 otherwise
 */
int testSolution(
    double ** const solution,
    const int solutionDimension,
    const double precision
)
{
    double newValue;

    for (int row = 1; row < solutionDimension - 1; row++) {
        for (int col = 1; col < solutionDimension - 1; col++) {
            newValue = (solution[row + 1][col] + solution[row - 1][col] +
                        solution[row][col + 1] + solution[row][col - 1]) / 4;

            if (fabs(newValue - solution[row][col]) >= precision) {
                return 0;
            }
        }
    }

    return 1;
}
