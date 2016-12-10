#include <math.h>

int testSolution(
    double ** const values,
    const int problemDimension,
    const double precision
)
{
    double newValue;

    for (int row = 1; row < problemDimension - 1; row++) {
        for (int col = 1; col < problemDimension - 1; col++) {
            newValue = (values[row + 1][col] + values[row - 1][col] +
                        values[row][col + 1] + values[row][col - 1]) / 4;

            if (fabs(newValue - values[row][col]) >= precision) {
                return 0;
            }
        }
    }

    return 1;
}
