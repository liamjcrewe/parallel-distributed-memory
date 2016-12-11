#include <math.h>
#include <string.h>
#include <mpi.h>
#include <stdio.h>

#include "../array/array.h"

/**
 * Relax a subset of rows in the updatedProblem array
 *
 * @param updatedProblem   The array to perform relaxation on
 * @param problemDimension The dimension of the problem array to perform
 *                         relaxation on
 * @param startRowIndex    The index of the first row to relax
 * @param rowsToRelax      The number of rows to relax
 * @param precision        The precision to relax values to
 */
static void relaxRows(
    double ** const updatedProblem,
    const int problemDimension,
    const int startRowIndex,
    const int rowsToRelax,
    const double precision
)
{
    double newValue;

    int lastRow = startRowIndex + rowsToRelax;

    // Do not try to relax fixed edge row, or any row past this
    if (lastRow > problemDimension - 1) {
        lastRow = problemDimension - 1;
    }

    // Skip first row
    int startRow = startRowIndex == 0 ? 1 : startRowIndex;

    for (int row = startRow; row < lastRow; row++) {
        for (int col = 1; col < problemDimension - 1; col++) {
            newValue = (updatedProblem[row + 1][col] +
                        updatedProblem[row - 1][col] +
                        updatedProblem[row][col + 1] +
                        updatedProblem[row][col - 1]) / 4;

            if (fabs(newValue - updatedProblem[row][col]) < precision) {
                continue;
            }

            updatedProblem[row][col] = newValue;
        }
    }
}

/**
 * Update the given problem to match the updatedProblem. Also checks if
 * any value changes as it does this. If no values changed in the last pass, we
 * know the solution is within precision, so we should terminate.
 *
 * @param  problem          The two dimensional problem array to update into
 * @param  updatedProblem   The two dimensional updatedProblem array to update
 *                          from
 * @param  problemDimension The dimension of the problem arrays
 *
 * @return                  1 if no update was made (problem is within
 *                          precision), 0 otherwise
 */
static int updateProblem(
    double ** const problem,
    double ** const updatedProblem,
    const int problemDimension
)
{
    int solved = 1;

    for (int row = 1; row < problemDimension - 1; row++) {
        for (int col = 1; col < problemDimension - 1; col++) {
            if (problem[row][col] == updatedProblem[row][col]) {
                continue;
            }

            problem[row][col] = updatedProblem[row][col];

            if (solved) {
                solved = 0;
            }
        }
    }

    return solved;
}

/**
 * Solve the given problem to the given precision in parallel, using the given
 * number of processors.
 *
 * @param  problem          The problem to solve (including padding rows)
 * @param  problemRows      The rows of the given problem array that are part
 *                          of the problem
 * @param  totalRows        The total rows of the given problem array
 * @param  precision        The precision to solve the problem to
 * @param  numProcessors    The number of processors being used to solve the
 *                          problem
 * @param  rowsPerProcessor The rows each processor should relax
 * @param  rank             The rank of the calling processor
 * @param  running_comm     Communicator containing all processes that are
 *                          running this function
 *
 * @return                  0 if success, error code otherwise
 */
int solve(
    double ** const problem,
    const int problemDimension,
    const int totalRows,
    const double precision,
    const int numProcessors,
    const int rowsPerProcessor,
    const int rank,
    MPI_Comm running_comm
)
{
    double ** const updatedProblem = createTwoDDoubleArray(
        totalRows,
        problemDimension
    );

    for (int i = 0; i < totalRows; i++) {
        for (int j = 0; j < problemDimension; j++) {
            updatedProblem[i][j] = problem[i][j];
        }
    }

    int displs[numProcessors];
    int sendCounts[numProcessors];

    for (int i = 0; i < numProcessors; i++) {
        sendCounts[i] = 1;
        displs[i] = i * rowsPerProcessor;
    }

    const int startRowIndex = rank * rowsPerProcessor;

    /* create a datatype to describe the subarrays of the global array */
    int sizes[2]    = {totalRows, problemDimension};                 /* global size */
    int subsizes[2] = {rowsPerProcessor, problemDimension};     /* local size */
    int starts[2]   = {0, 0};                /* where this one starts */
    MPI_Datatype type, subarrtype;

    int error;

    error = MPI_Type_create_subarray(2, sizes, subsizes, starts, MPI_ORDER_C, MPI_DOUBLE, &type);
    // Set extent to one row
    error = MPI_Type_create_resized(type, 0, problemDimension * sizeof(double), &subarrtype);
    error = MPI_Type_commit(&subarrtype);

    if (error) {
        return error;
    }

    int solved = 0;

    while (!solved) {
        // startRowIndex is different for each process
        relaxRows(
            updatedProblem,
            problemDimension,
            startRowIndex,
            rowsPerProcessor,
            precision
        );

        error = MPI_Allgatherv(
            updatedProblem[startRowIndex],
            rowsPerProcessor * problemDimension,
            MPI_DOUBLE,
            updatedProblem[0],
            sendCounts,
            displs,
            subarrtype,
            running_comm
        );

        if (error) {
            return error;
        }

        solved = updateProblem(problem, updatedProblem, problemDimension);
    }

    freeTwoDDoubleArray(updatedProblem);

    MPI_Type_free(&subarrtype);

    return 0;
}
