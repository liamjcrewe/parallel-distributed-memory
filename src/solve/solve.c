#include <math.h>
#include <string.h>
#include <mpi.h>
#include <stdio.h>

#include "../array/array.h"

static void relaxRows(
    double ** const values,
    const int problemRows,
    const int cols,
    const int startRowIndex,
    const int rowsToRelax,
    const double precision
)
{
    int rowIndex;
    double newValue;

    for (int row = startRowIndex; row < startRowIndex + rowsToRelax; row++) {
        rowIndex = (int) values[row][cols - 1];
        // Skip first and last row
        if (rowIndex == 0 || rowIndex == problemRows - 1) {
            continue;
        }

        for (int col = 1; col < cols - 2; col++) {
            newValue = (values[row + 1][col] + values[row - 1][col]
                        + values[row][col + 1] + values[row][col - 1]) / 4;

            if (fabs(newValue - values[row][col]) < precision) {
                continue;
            }

            values[row][col] = newValue;
        }
    }
}

static int updateValues(
    double ** const values,
    double ** const newValues,
    const int problemRows,
    const int cols
)
{
    int rowIndex;
    int solved = 1;

    for (int row = 1; row < problemRows - 1; row++) {
        rowIndex = (int) newValues[row][cols - 1];

        // Skip padding rows
        if (rowIndex == 0) {
            break;
        }

        for (int col = 1; col < cols - 2; col++) {
            if (values[rowIndex][col] == newValues[row][col]) {
                continue;
            }

            values[rowIndex][col] = newValues[row][col];

            if (solved) {
                solved = 0;
            }
        }
    }

    return solved;
}

int solve(
    double ** const values,
    const int problemRows,
    const int totalRows,
    const int cols,
    const double precision,
    const int numProcessors,
    const int rowsPerProcessor,
    const int rank,
    MPI_Comm running_comm
)
{
    double ** const newValues = createTwoDDoubleArray(totalRows, cols);

    for (int i = 0; i < totalRows; i++) {
        for (int j = 0; j < cols; j++) {
            newValues[i][j] = values[i][j];
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
    int sizes[2]    = {totalRows, cols};                 /* global size */
    int subsizes[2] = {rowsPerProcessor, cols};     /* local size */
    int starts[2]   = {0, 0};                /* where this one starts */
    MPI_Datatype type, subarrtype;

    int error;

    error = MPI_Type_create_subarray(2, sizes, subsizes, starts, MPI_ORDER_C, MPI_DOUBLE, &type);
    // Set extent to one row
    error = MPI_Type_create_resized(type, 0, cols * sizeof(double), &subarrtype);
    error = MPI_Type_commit(&subarrtype);

    if (error) {
        return error;
    }

    int solved = 0;

    while (!solved) {
        // startRowIndex is different for each process
        relaxRows(
            newValues,
            problemRows,
            cols,
            startRowIndex,
            rowsPerProcessor,
            precision
        );

        error = MPI_Allgatherv(
            newValues[startRowIndex],
            rowsPerProcessor * cols,
            MPI_DOUBLE,
            newValues[0],
            sendCounts,
            displs,
            subarrtype,
            running_comm
        );

        if (error) {
            return error;
        }

        solved = updateValues(values, newValues, totalRows, cols);
    }

    freeTwoDDoubleArray(newValues, totalRows);

    MPI_Type_free(&subarrtype);

    return 0;
}
