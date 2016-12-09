#include <math.h>
#include <string.h>
#include <mpi.h>
#include <stdio.h>

#include "../array/array.h"

static void relaxRows(
    double ** const newValues,
    const int problemDimension,
    const int startRowIndex,
    const int rowsToRelax,
    const double precision
)
{
    double newValue;

    for (int row = startRowIndex; row < startRowIndex + rowsToRelax; row++) {
        // Skip first and last row
        if (row == 0 || row == problemDimension - 1) {
            continue;
        }

        for (int col = 1; col < problemDimension - 1; col++) {
            newValue = (newValues[row + 1][col] + newValues[row - 1][col] +
                        newValues[row][col + 1] + newValues[row][col - 1]) / 4;

            if (fabs(newValue - newValues[row][col]) < precision) {
                continue;
            }

            newValues[row][col] = newValue;
        }
    }
}

static int updateValues(
    double ** const values,
    double ** const newValues,
    const int problemDimension
)
{
    int solved = 1;

    for (int row = 1; row < problemDimension - 1; row++) {
        for (int col = 1; col < problemDimension - 1; col++) {
            if (values[row][col] == newValues[row][col]) {
                continue;
            }

            values[row][col] = newValues[row][col];

            if (solved) {
                solved = 0;
            }
        }
    }

    return solved;
}

int solve(
    double ** const values,
    const int problemDimension,
    const int totalRows,
    const double precision,
    const int numProcessors,
    const int rowsPerProcessor,
    const int rank,
    MPI_Comm running_comm
)
{
    double ** const newValues = createTwoDDoubleArray(totalRows, problemDimension);

    for (int i = 0; i < totalRows; i++) {
        for (int j = 0; j < problemDimension; j++) {
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
            newValues,
            problemDimension,
            startRowIndex,
            rowsPerProcessor,
            precision
        );

        error = MPI_Allgatherv(
            newValues[startRowIndex],
            rowsPerProcessor * problemDimension,
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

        solved = updateValues(values, newValues, problemDimension);
    }

    freeTwoDDoubleArray(newValues, totalRows);

    MPI_Type_free(&subarrtype);

    return 0;
}
