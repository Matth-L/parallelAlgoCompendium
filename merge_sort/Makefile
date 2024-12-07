all:
	gcc -Wall -Wextra -g -fopenmp sequential.c -o sequential
	gcc -Wall -Wextra -g -fopenmp -lpthread pthread.c -o pthread 
	gcc -Wall -Wextra -g -fopenmp openmp.c -o openmp

test : 
	make all 
	touch results.txt
	./create_array.sh 20 
	./sequential unsorted_array_20.txt results.txt
	export OMP_NUM_THREADS=48; ./pthread unsorted_array_20.txt results.txt
	export OMP_NUM_THREADS=48; ./openmp unsorted_array_20.txt results.txt

benchmark_sequential:
	make all
	@echo "Benchmarking sequential"
	n=2; \
	while [ "$$n" -lt 20000000 ]; do \
		echo "n = $$n"; \
		./sequential $$n; \
		n=$$(( n * 2 )); \
	done

benchmark_pthread:
	make all
	@echo "Benchmarking pthread"
	n=2; \
	while [ "$$n" -lt 200000000 ]; do \
		echo "n = $$n"; \
		export OMP_NUM_THREADS=48; ./pthread $$n; \
		n=$$(( n * 2 )); \
	done

benchmark_openmp:
	make all
	@echo "Benchmarking openmp"
	n=2; \
	while [ "$$n" -lt 200000000 ]; do \
		echo "n = $$n"; \
		export OMP_NUM_THREADS=48; ./openmp $$n; \
		n=$$(( n * 2 )); \
	done

clean : 
	rm -fv a.out
	rm -fv pthread openmp sequential
	rm *.txt

find_n :
	gcc find_n.c -fopenmp -lpthread -o find_n