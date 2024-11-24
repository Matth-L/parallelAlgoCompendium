all:
	mpicc matth_parallel_eratosthene_test.c -lm
	mpirun -np 3 a.out 25
	rm a.out
