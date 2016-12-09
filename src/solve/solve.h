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
