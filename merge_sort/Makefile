all:
	gcc -Wall -Wextra -g -fopenmp d2s.c -o d2s
	gcc -Wall -Wextra -g -fopenmp -lpthread d2p.c -o d2p 
	gcc -Wall -Wextra -g -fopenmp d2omp.c -o d2omp
	touch results.txt

test : 
	make all 
	./create_array.sh 20 
	./d2s unsorted_array_20.txt results.txt
	export OMP_NUM_THREADS=48; ./d2p unsorted_array_20.txt results.txt

benchmark_sequential:
	make all
	@echo "Benchmarking d2s"
	n=2; \
	while [ "$$n" -lt 20000000 ]; do \
		echo "n = $$n"; \
		./d2s $$n; \
		n=$$(( n * 2 )); \
	done

benchmark_pthread:
	make all
	@echo "Benchmarking d2p"
	n=2; \
	while [ "$$n" -lt 200000000 ]; do \
		echo "n = $$n"; \
		export OMP_NUM_THREADS=48; ./d2p $$n; \
		n=$$(( n * 2 )); \
	done

benchmark_openmp:
	make all
	@echo "Benchmarking d2omp"
	n=2; \
	while [ "$$n" -lt 200000000 ]; do \
		echo "n = $$n"; \
		export OMP_NUM_THREADS=48; ./d2omp $$n; \
		n=$$(( n * 2 )); \
	done
clean : 
	rm -fv a.out
	rm -fv d2p d2omp d2s
	rm *.txt

