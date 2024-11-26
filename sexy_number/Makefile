p1:
	mpicc d3-1.c -lm -fopenmp
	mpirun -np 2 ./a.out 10000
	rm a.out
p2:
	mpicc d3-2.c -lm -fopenmp
	mpirun -np 2 ./a.out 10000
	rm a.out

clean:
	rm a.out
