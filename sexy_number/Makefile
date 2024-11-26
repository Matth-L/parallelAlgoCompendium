p1:
	mpicc d3-1.c -lm -fopenmp
	mpirun -np 12 ./a.out 64 
	rm a.out
p2:
	mpicc d3-2.c -lm -fopenmp
	mpirun -np 12 ./a.out 64
	rm a.out
clean:
	rm a.out
