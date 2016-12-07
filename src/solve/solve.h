int solve(
    double ** const values,
    const int rows,
    const int cols,
    const double precision,
    const int numProcessors,
    const int rowsPerProcessor,
    const int rank,
    MPI_Comm running_comm
);
