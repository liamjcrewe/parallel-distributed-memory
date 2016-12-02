#include <math.h>
#include <string.h>
#include <mpi.h>

#include "../array/array.h"

static void setRowsPerProcessor(
    const int rows,
    const int numProcesses,
    int * const rowsPerProcess
)
{
    for (int count = 0; count < rows; count++) {
        rowsPerProcess[count % numProcesses] += 1;
    }
}

static void relaxRows(
    double ** const values,
    const int rows,
    const int cols,
    const int startRow,
    const int rowsToRelax,
    const int precision
)
{
    int rowIndex;
    int newValue;

    for (int row = startRow; row < startRow + rowsToRelax; row++) {
        rowIndex = (int) values[row][cols];
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
    const int cols,
    const double precision
)
{
    int rowIndex;
    int solved = 1;

    for (int row = 1; row < rows - 1; row++) {
        rowIndex = (int) newValues[row][cols - 1];

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
    const int numProcesses,
    const int processId
)
{
    double ** const newValues = createTwoDDoubleArray(rows, cols);
    int rowsPerProcess[numProcesses];
    memset(rowsPerProcess, 0, sizeof rowsPerProcess);

    setRowsPerProcessor(rows, numProcesses, rowsPerProcess);

    int displs[numProcesses];
    int sum = 0;
    for (int i = 0; i < numProcesses; i++) {
        displs[i] += sum;
        sum += rowsPerProcess[i];
    }

    const int startRow = displs[processId];

    /* create a datatype to describe the subarrays of the global array */
    int sizes[2]    = {rows, cols};         /* global size */
    int subsizes[2] = {rowsPerProcess[numProcesses], cols};     /* local size */
    int starts[2]   = {0, 0};                        /* where this one starts */
    MPI_Datatype type, subarrtype;
    MPI_Type_create_subarray(2, sizes, subsizes, starts, MPI_ORDER_C, MPI_DOUBLE, &type);
    // set extent to one row
    MPI_Type_create_resized(type, 0, cols*sizeof(double), &subarrtype);
    MPI_Type_commit(&subarrtype);

    int solved;
    while (!solved) {
        solved = 1;

        // startRow and rowsPerProcess[processId] are different for each process
        relaxRows(
            values,
            rows,
            cols,
            startRow,
            rowsPerProcess[processId],
            precision
        );

        MPI_Allgatherv(
            &(values[0][0]),
            rowsPerProcess[processId] * cols,
            MPI_DOUBLE,
            newValues,
            rowsPerProcess,
            displs,
            subarrtype,
            MPI_COMM_WORLD
        );

        solved = updateValues(values, newValues, rows, cols, precision);

        solved = 1;
        // print newValues out somehow to see what the hell it is.

        //allgatherv into newValues
        //
        //loop over newValues
        //compare each row to corresponding row in values
        //if change {
        //   update values
        //
        //   if solved {
        //     set solved to 0
        //   }
        //}
    }

    freeTwoDDoubleArray(newValues, rows);

    return 0;
}
