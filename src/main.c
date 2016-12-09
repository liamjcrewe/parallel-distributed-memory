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

#define INVALID_NUM_ARGS "You must specify problem dimension and precision.\n"

#define INVALID_PROBLEM_DIMENSION "Invalid problem dimension given. "\
                                  "Must be an integer greater than 0.\n"

#define INVALID_PRECISION "Precision must be a number greater than 0\n"

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

static int roundToMultiple(const int input, const int multiple)
{
    const int remainder = input % multiple;

    if (!remainder) {
        return input;
    }

    return input + multiple - remainder;
}

static int runSolve(
    const int problemDimension,
    const double precision,
    int initialNumProcessors,
    const int rank
)
{
    int numProcessors = initialNumProcessors;

    // This separates problem by rows, so cannot use more processes than rows
    if (numProcessors > problemDimension) {
        numProcessors = problemDimension;
    }

    int leftoverRows = problemDimension % numProcessors;
    int totalRows = problemDimension;

    int rowsPerProcessor = (problemDimension - leftoverRows) / numProcessors;

    // if number of rows not divisible by number of processors, we need to pad
    if (leftoverRows) {
        // we have leftover rows, so need to do one more row per processor
        rowsPerProcessor++;

        // Round number of rows to a multiple of rowsPerProcessor
        totalRows = roundToMultiple(problemDimension, rowsPerProcessor);

        // Only use enough processors to cover all the rows
        numProcessors = totalRows / rowsPerProcessor;
    }

    int shouldRun = rank < numProcessors;

    MPI_Comm running_comm;
    MPI_Comm_split(MPI_COMM_WORLD, shouldRun, rank, &running_comm);

    // Not using these processors, so just return
    if (!shouldRun) {
        return 0;
    }

    // Create problem array, including padding rows
    double ** const values = createTwoDDoubleArray(totalRows, problemDimension);
    // Load problem into problem array
    const int result = fillProblemArray(values, problemDimension);

    if (result == -1) {
        if (isMainThread(rank)) {
            printf(INVALID_PROBLEM_DIMENSION);
        }

        return -1;
    }

    // Fill padding rows with 0.0
    for (int row = problemDimension; row < totalRows; row++) {
        memset(values[row], 0.0, problemDimension);
    }

    FILE * f;

    if (isMainThread(rank)) {
        char fileName[80];
        sprintf(
            fileName,
            "./output/output-%d-%g-%d.txt",
            problemDimension,
            precision,
            initialNumProcessors
        );

        f = fopen(fileName, "w");

        // Log input
        fprintf(f, "Input:\n");
        write2dDoubleArray(f, values, problemDimension);
    }

    int error = solve(
        values,
        problemDimension,
        totalRows,
        precision,
        numProcessors,
        rowsPerProcessor,
        rank,
        running_comm
    );

    if (error) {
        /*
         * Print but don't return. We still want to output whatever solution we
         * got to below, and clean up memory and file handle.
         */
        printf(ERROR, error);
    }

    if (isMainThread(rank)) {
        // Log solution
        fprintf(f, "Solution:\n");
        write2dDoubleArray(f, values, problemDimension);

        fclose(f);
    }

    // Free memory
    freeTwoDDoubleArray(values, totalRows);

    MPI_Comm_free(&running_comm);

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

        MPI_Finalize();

        return error;
    }

    int numProcessors, rank;

    error = MPI_Comm_size(MPI_COMM_WORLD, &numProcessors);

    if (error) {
        printf(MPI_ERROR, error);

        MPI_Finalize();

        return error;
    }

	error = MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (error) {
        printf(MPI_ERROR, error);

        MPI_Finalize();

        return error;
    }

    // Help CLI
    if (helpFlagSet(argc, argv)) {
        if (isMainThread(rank)) {
            printf(HELP);
        }

        MPI_Finalize();

        return 0;
    }

    // Parse and validate args
    if (argc != 3) {
        if (isMainThread(rank)) {
            printf(INVALID_NUM_ARGS);
        }

        MPI_Finalize();

        return -1;
    }

    const int problemDimension = atoi(argv[1]);
    const double precision = atof(argv[2]);

    if (problemDimension <= 0) {
        if (isMainThread(rank)) {
            printf(INVALID_PROBLEM_DIMENSION);
        }

        MPI_Finalize();

        return -1;
    }

    if (precision <= 0) {
        if (isMainThread(rank)) {
            printf(INVALID_PRECISION);
        }

        MPI_Finalize();

        return -1;
    }

    // Solve and clean up
    int res = runSolve(problemDimension, precision, numProcessors, rank);

    if (res) {
        printf(MPI_ERROR, res);

        return res;
    }

    error = MPI_Finalize();

    if (error) {
        printf(MPI_ERROR, error);

        return error;
    }

    return res;
}
