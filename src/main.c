#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#include "array/array.h"
#include "output/output.h"
#include "problem/problem.h"
#include "solve/solve.h"

#define HELP "Argument order:\n"\
             " - Problem ID (1, 2, 3, 4, 5 or 6. See src/problem/problem.c).\n"\
             " - Precision to work to.\n"

#define INVALID_NUM_ARGS "You must specify problem ID, "\
                         "number of threads and precision.\n"

#define INVALID_PROBLEM_ID "Invalid problem id given. "\
                           "Must be 1, 2, 3, 4, 5 or 6.\n"

#define INVALID_PRECISION "Precision must be a decimal greater than 0\n"

#define ERROR "Something went wrong. Error code: %d\n"

#define MPI_ERROR "Something went wrong with MPI. Error code: %d\n"

/**
 * Checks if the given processorId is the 'main' thread (rank 0)
 *
 * @param  rank   The id of the process to check
 *
 * @return             1 if main (rank is 0), 0 otherwise
 */
static int isMainThread(const int rank)
{
    return rank == 0;
}

/**
 * Checks if any of the parameters passed via CLI are --help or -h
 *
 * @param  argc Number of command line argmuments
 * @param  argv Array of command line arguments
 *
 * @return      1 if true (help flag specified), 0 otherwise
 */
static int helpFlagSet(int argc, char *argv[])
{
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0
            || strcmp(argv[i], "-h") == 0) {

            return 1;
        }
    }

    return 0;
}

static int runSolve(
    const int problemId,
    const double precision,
    const int numProcessors,
    const int rank
)
{
    const int rows = getProblemRows(problemId);
    const int cols = rows + 1;

    double ** const values = createTwoDDoubleArray(rows, cols);
    const int result = fillProblemArray(values, problemId);

    if (result == -1) {
        if (isMainThread(rank)) {
            printf(INVALID_PROBLEM_ID);
        }

        return -1;
    }

    FILE * f;

    if (isMainThread(rank)) {
        f = fopen("./output.txt", "w");

        // Log input
        fprintf(f, "Input:\n");
        write2dDoubleArray(f, values, rows);
    }

    /**** add index of each row as last element of problem array ****/
    int error = solve(
        values,
        rows,
        cols,
        precision,
        numProcessors,
        rank
    );

    if (error) {
        printf(ERROR, error);
    }

    if (isMainThread(rank)) {
        // Log solution
       fprintf(f, "Solution:\n");
       write2dDoubleArray(f, values, rows);

       fclose(f);
    }

    // Free memory
    freeTwoDDoubleArray(values, rows);

    return error ? error : 0;
}

/**
 * Main function. Runs simple CLI tool that allows --help/-h, and reports an
 * error if not enough/too many command line parameters are passed.
 *
 * Initialises and sets up MPI, and calls runSolve to start solving of the
 * given problem.
 *
 * @param  argc Number of command line arguments
 * @param  argv Array of command line arguments
 * @return      0 if success, -1 if error.
 */

int main(int argc, char *argv[])
{
    int error;

    // Init MPI and set up
    error = MPI_Init(&argc, &argv);

    if (error) {
        printf(MPI_ERROR, error);

        MPI_Abort(MPI_COMM_WORLD, error);
    }

    int numProcessors, rank;

    error = MPI_Comm_size(MPI_COMM_WORLD, &numProcessors);

    if (error) {
        printf(MPI_ERROR, error);

        MPI_Abort(MPI_COMM_WORLD, error);
    }

	error = MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (error) {
        printf(MPI_ERROR, error);

        MPI_Abort(MPI_COMM_WORLD, error);
    }

    if (helpFlagSet(argc, argv)) {
        if (isMainThread(rank)) {
            printf(HELP);
        }

        MPI_Finalize();

        return 0;
    }

    if (argc != 3) {
        if (isMainThread(rank)) {
            printf(INVALID_NUM_ARGS);
        }

        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    const int problemId = atoi(argv[1]);
    const double precision = atof(argv[2]);

    if (problemId <= 0) {
        if (isMainThread(rank)) {
            printf(INVALID_PROBLEM_ID);
        }

        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    if (precision <= 0) {
        if (isMainThread(rank)) {
            printf(INVALID_PRECISION);
        }

        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    int res = runSolve(problemId, precision, numProcessors, rank);

    error = MPI_Finalize();

    if (error) {
        printf(MPI_ERROR, error);

        MPI_Abort(MPI_COMM_WORLD, error);
    }

    return res;
}
