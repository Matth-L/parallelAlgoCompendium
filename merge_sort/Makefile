all:
	gcc -Wall -Wextra -g -fopenmp -lpthread d2p.c -o d2p 
	gcc -Wall -Wextra -g -fopenmp d2omp.c -o d2omp


benchmark:
	make all
	@echo "Benchmarking d2p and d2omp"
	n=4; \
	while [ "$$n" -lt 100000000 ]; do \
		n=$$(( n * 2 )); \
		echo "n = $$n"; \
		./d2p $$n; \
		./d2omp $$n; \
	done

clean : 
	rm -fv a.out
	rm -fv d2p d2omp

