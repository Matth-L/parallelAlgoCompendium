p1:
	mpicc d3-1.c -lm -fopenmp
	mpirun -np 4 ./a.out 10000000000
	rm a.out
p2:
	mpicc d3-2.c -lm -fopenmp
	mpirun -np 4 ./a.out 10000000000
	rm a.out
clean:
	rm a.out
