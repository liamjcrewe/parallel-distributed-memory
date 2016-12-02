all:
	mpicc -std=c99 src/**/*.c src/main.c -o bin/solve
debug:
	mpicc -std=c99 src/**/*.c src/main.c -Wall -o bin/solve
clean:
	rm bin/solve; rm -rf bin/solve.dSYM/
