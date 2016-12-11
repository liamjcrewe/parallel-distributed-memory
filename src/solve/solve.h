/**
 * Solve the given problem (values) to the given precision in parallel, using
 * the given number of processors.
 *
 * @param  values           The problem to solve (including padding rows)
 * @param  problemRows      The rows of the given values array that are part of
 *                          the problem
 * @param  totalRows        The total rows of the given values array
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
    double ** const values,
    const int problemRows,
    const int totalRows,
    const double precision,
    const int numProcessors,
    const int rowsPerProcessor,
    const int rank,
    MPI_Comm running_comm
);
