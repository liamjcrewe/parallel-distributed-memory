# parallel-distributed-memory

Coursework completed as part of 'Parallel Programming' final year module for BSc Computer Science at University of Bath.

## Compiling
Running ```make``` will compile to bin/solve.

Other targets are:
* debug (turn warnings and debugging output on)
* clean (remove compiled code, and output files)

## Running
### Basic operation
Run ```mpirun -np [processors] bin/solve [problem-dimension] [precision]```. This program allows any size problem to be generated and solved.

After running, both the input and the solution are written to ```output/solution-[problem-dimension]-[precision]-[processors].txt```

### Help
Run ```bin/solve [--help|-h]``` for help.

### Test
Run ```mpirun -np [processors] bin/solve [problem-dimension] [precision] [--test|-t]```. This tests the achieved solution to check that it is within precision. Results are written to ```output/test-[problem-dimension]-[precision]-[processors].txt```
