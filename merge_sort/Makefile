all : 
	make dir
	make 

dir : 
	mkdir -p bin
	mkdir -p results

test:
	@echo "Running tests"
	@echo "Test pthread with 16 threads"
	gcc 16 ./src/d2p.c -lpthread -lgomp
	touch resultat
	./a.out 16 ./results/unsorted_array_100000.txt resultat
	gcc ./src/d2omp.c -fopenmp
	./a.out 16 ./results/unsorted_array_100000.txt resultat

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

clean :
	rm a.out
	rm -rf bin
