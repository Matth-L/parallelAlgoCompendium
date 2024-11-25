all:
	mpicc d3-1.c -lm -fopenmp
	mpirun -np 4 a.out 25
	rm a.out
