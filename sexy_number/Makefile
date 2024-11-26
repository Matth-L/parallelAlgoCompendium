all:
	mpicc d3-2.c -lm -fopenmp
	mpirun -np 2 ./a.out 10000
	rm a.out
