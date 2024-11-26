all:
	mpicc d3-1.c -lm -fopenmp
	mpirun -np 1 ./a.out 100000
	rm a.out
