#include <math.h>
#include <string.h>
#include <mpi.h>
#include <stdio.h>

#include "../array/array.h"

static void relaxRows(
    double ** const values,
    const int rows,
    const int cols,
    const int startRowIndex,
    const int rowsToRelax,
    const int precision
)
{
    int rowIndex;
    double newValue;

    for (int row = startRowIndex; row < startRowIndex + rowsToRelax; row++) {
        rowIndex = (int) values[row][cols - 1];
        if (rowIndex == 0 || rowIndex == rows - 1) {
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
    const int rows,
    const int cols
)
{
    int rowIndex;
    int solved = 1;

    for (int row = 1; row < rows - 1; row++) {
        rowIndex = (int) newValues[row][cols - 1];

        // Skip padding rows
        if (rowIndex == 0) {
            break;
        }

        if (doubleArraysEqual(values[rowIndex], newValues[row], cols)) {
            continue;
        }

        for (int col = 1; col < cols - 2; col++) {
            values[rowIndex][col] = newValues[row][col];
        }

        if (solved) {
            solved = 0;
        }
    }

    return solved;
}

int solve(
    double ** const values,
    const int rows,
    const int cols,
    const double precision,
    const int numProcessors,
    const int rowsPerProcessor,
    const int rank,
    MPI_Comm running_comm
)
{
    double ** const newValues = createTwoDDoubleArray(rows, cols);

    int displs[numProcessors];
    int sendCounts[numProcessors];

    for (int i = 0; i < numProcessors; i++) {
        sendCounts[i] = 1;
        displs[i] = i * rowsPerProcessor;
    }

    const int startRowIndex = rank * rowsPerProcessor;

    /* create a datatype to describe the subarrays of the global array */
    int sizes[2]    = {rows, cols};                 /* global size */
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
            values,
            rows,
            cols,
            startRowIndex,
            rowsPerProcessor,
            precision
        );

        error = MPI_Allgatherv(
            values[startRowIndex],
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

        solved = updateValues(values, newValues, rows, cols);

        MPI_Comm_split(running_comm, solved, rank, &running_comm);
    }

    freeTwoDDoubleArray(newValues, rows);

    MPI_Type_free(&subarrtype);

    return 0;
}
