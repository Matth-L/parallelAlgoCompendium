all :
	make p1
	make p2

p1: send_rcv
	mpirun -np 4 ./send_rcv 1000
	rm send_rcv

p2: get_put
	mpirun -np 4 ./get_put 1000
	rm get_put

send_rcv: send_rcv.c
	mpicc send_rcv.c -o send_rcv -lm -fopenmp

get_put: get_put.c
	mpicc get_put.c -o get_put -lm -fopenmp

clean:
	rm -v send_rcv get_put
