all:
	gcc -Wall -Wextra -g -fopenmp -lpthread d2p.c -o d2p 
	gcc -Wall -Wextra -g -fopenmp d2omp.c -o d2omp



benchmark_pthread:
	make all
	@echo "Benchmarking d2p and d2omp"
	n=2; \
	while [ "$$n" -lt 200000000 ]; do \
		echo "n = $$n"; \
		./d2p $$n; \
		n=$$(( n * 2 )); \
	done

benchmark_openmp:
	make all
	@echo "Benchmarking d2p and d2omp"
	n=2; \
	while [ "$$n" -lt 200000000 ]; do \
		echo "n = $$n"; \
		./d2omp $$n; \
		n=$$(( n * 2 )); \
	done
clean : 
	rm -fv a.out
	rm -fv d2p d2omp

