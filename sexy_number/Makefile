all:
	mpicc matth_parallel_eratosthene_test.c -lm
	mpirun -np 4  a.out 64
	rm a.out
