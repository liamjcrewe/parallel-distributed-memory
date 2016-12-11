#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#include "array/array.h"
#include "problem/problem.h"
#include "solve/solve.h"
#include "test/test.h"

#define HELP "Argument order:\n"\
             " - Problem dimension (integer > 0).\n"\
             " - Precision to work to (number > 0).\n"\
             " - Optional: [--test|-t] to test achieved solution.\n"

#define INVALID_NUM_ARGS "You must specify problem dimension and precision.\n"

#define INVALID_PROBLEM_DIMENSION "Invalid problem dimension given. "\
                                  "Must be an integer greater than 0.\n"

#define INVALID_PRECISION "Invalid precision given. "\
                          "Must be a number greater than 0\n"

#define ERROR "Something went wrong. Error code: %d\n"

#define MPI_ERROR "Something went wrong with MPI. Error code: %d\n"

/**
 * Checks if the given processorId is the 'main' thread (rank 0).
 *
 * @param  rank   The id of the process to check
 *
 * @return        1 if main (rank is 0), 0 otherwise
 */
static int isMainThread(const int rank)
{
    return rank == 0;
}

/**
 * Checks if any of the parameters passed via CLI are --help or -h.
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

/**
 * Checks if any of the parameters passed via CLI are --test or -t.
 *
 * @param  argc Number of command line argmuments
 * @param  argv Array of command line arguments
 *
 * @return      1 if true (test flag specified), 0 otherwise
 */
static int testFlagSet(int argc, char *argv[])
{
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--test") == 0
            || strcmp(argv[i], "-t") == 0) {

            return 1;
        }
    }

    return 0;
}

/**
 * Round input to the first value greater than input that is divisble by given
 * multiple.
 *
 * @param  input    Value to round
 * @param  multiple Value that returned value should be divisible by
 *
 * @return          First value greater than input that is divisible by given
 *                  multiple
 */
static int roundToMultiple(const int input, const int multiple)
{
    const int remainder = input % multiple;

    if (!remainder) {
        return input;
    }

    return input + multiple - remainder;
}

/**
 * Write a two dimensional array of doubles to a given file.
 *
 * @param f                File handle to write to
 * @param array            Two dimensional array of doubles to write to file
 * @param problemDimension Dimension of array
 */
void write2dDoubleArray(
    FILE * const f,
    double ** const array,
    const int problemDimension
)
{
    for (int row = 0; row < problemDimension; ++row) {
        for (int col = 0; col < problemDimension; ++col) {
            fprintf(f, "%10f ", array[row][col]);
        }
        fputs("\n", f);
    }
}

/**
 * Generate, set up and run solve on a problem of problemDimension size to the
 * given precision.
 *
 * Carries out various set up features such as selecting optimimum number of
 * processors, padding of problem so that every processor can be assigned the
 * same amount of rows. Also outputs solution to file, and allows solution to
 * be tested (and the result written to file) for correctness testing.
 *
 * @param  problemDimension  Dimension of problem to generate and solve
 * @param  precision         Precision to solve generated problem to
 * @param  maxProcessors     Maximum number of processors allows
 * @param  rank              Rank of processor calling this function
 * @param  test              Flag to say whether to test the solution and write
 *                           test result to file
 *
 * @return                   0 if success, error code otherwise
 */
static int runSolve(
    const int problemDimension,
    const double precision,
    int maxProcessors,
    const int rank,
    const int test
)
{
    int numProcessors = maxProcessors;

    // This separates problem by rows, so cannot use more processes than rows
    if (numProcessors > problemDimension) {
        numProcessors = problemDimension;
    }

    // See if rows is divisible by number of processors
    int leftoverRows = problemDimension % numProcessors;
    int totalRows = problemDimension;

    int rowsPerProcessor = (problemDimension - leftoverRows) / numProcessors;

    // If number of rows not divisible by number of processors, we need to pad
    if (leftoverRows) {
        // We have leftover rows, so need to do one more row per processor
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
    fillProblemArray(values, problemDimension);

    // Fill padding rows with 0.0
    for (int row = problemDimension; row < totalRows; row++) {
        memset(values[row], 0.0, problemDimension);
    }

    FILE * f;

    // Open solution file and write input problem to file
    if (isMainThread(rank)) {
        char fileName[80];
        sprintf(
            fileName,
            "./output/solution-%d-%g-%d.txt",
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

    // Write solution to file
    if (isMainThread(rank)) {
        // Log solution
        fprintf(f, "Solution:\n");
        write2dDoubleArray(f, values, problemDimension);

        fclose(f);
    }

    // Test result and write result to file
    if (test) {
        char fileName[80];
        sprintf(
            fileName,
            "./output/test-%d-%g-%d.txt",
            problemDimension,
            precision,
            initialNumProcessors
        );

        FILE * testFile = fopen(fileName, "w");

        int res = testSolution(values, problemDimension, precision);

        // Log input
        fprintf(
            testFile,
            "Dimension: %d, Precision: %g, Processors: %d, Result: %s.\n",
            problemDimension,
            precision,
            initialNumProcessors,
            res ? "Pass" : "Fail"
        );

        fclose(testFile);
    }

    // Free memory
    freeTwoDDoubleArray(values);

    MPI_Comm_free(&running_comm);

    return error ? error : 0;
}

/**
 * Main function. Runs simple CLI too with help/error checking, and parses
 * CLI arguments.
 *
 * Initialises and sets up MPI, and calls runSolve to generate and solve the
 * problem.
 *
 * @param  argc Number of command line arguments
 * @param  argv Array of command line arguments
 * @return      0 if success, error code otherwise
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

    int test = 0;

    // Test flag
    if (testFlagSet(argc, argv) && isMainThread(rank)) {
        test = 1;
    }

    // Parse and validate args
    if (argc < 3) {
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
    int res = runSolve(problemDimension, precision, numProcessors, rank, test);

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
