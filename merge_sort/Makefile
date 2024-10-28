all : 
	make dir
	make test

dir : 
	mkdir -p bin
	mkdir -p results

test:
	@echo "Running tests"
	touch resultat_seq resultat_pth resultat_omp
	@echo "Test sequential"
	gcc ./src/d2s.c -lgomp
	./a.out ./results/unsorted_array_100000.txt ./resultat_seq
	@echo "Test pthread with 16 threads"
	gcc ./src/d2p.c -lpthread -lgomp
	./a.out 16 ./results/unsorted_array_100000.txt ./resultat_pth
	@echo "Test openMP with 16 threads"
	gcc ./src/d2omp.c -fopenmp
	./a.out 16 ./results/unsorted_array_100000.txt ./resultat_omp

array:
	@echo "creating array";
	for size in 10 10000 1000000 ; do \
		./src/create_array.sh $$size results; \
	done

style : 
	astyle -A1 *.c --recursive
	@if ls *.h >/dev/null 2>&1; then astyle -A1 *.h; fi
	find . -type f -name "*.orig" -delete

benchmark:
	@echo "Running benchmark"
	gcc -c ./src/utils.c -o ./bin/utils.o -lm -g -lgomp
	gcc ./bin/utils.o src/benchmark.c -fopenmp -lpthread -o ./bin/benchmark
	./bin/benchmark ./results/unsorted_array_100000.txt


clean:
	rm -fv a.out
	rm -fv ./bin/*
	rm -fv resultat_*
