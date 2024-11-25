all:
	mpicc correction1.c -lm -fopenmp
	mpirun -np 3 ./a.out 25
	mpirun -np 3 ./a.out 10000
	rm a.out
	mpicc d3-1.c -lm -fopenmp
	mpirun -np 3 ./a.out 25
	mpirun -np 3 ./a.out 10000
	rm a.out
