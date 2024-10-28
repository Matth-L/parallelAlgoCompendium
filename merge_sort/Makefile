all : 
	

dir : 
	mkdir -p bin
	mkdir -p results


compile:
	gcc -c ./src/utils.c -o ./bin/utils.o 
	gcc -c ./src/sequential_merge_sort.c -o ./bin/sequential_merge_sort.o -lm -g -lgomp
	gcc -c ./src/pthread_merge_sort.c -o ./bin/pthread_merge_sort.o -lm -lpthread -g -lgomp
	gcc -c ./src/openMP_merge_sort.c -o ./bin/openMP_merge_sort.o -lm -fopenmp -g -lgomp
	gcc -c ./src/benchmark.c -o ./bin/benchmark.o -lm -g -lgomp
	gcc ./bin/utils.o ./bin/sequential_merge_sort.o ./bin/pthread_merge_sort.o ./bin/openMP_merge_sort.o ./bin/benchmark.o -o ./bin/benchmark -lm -fopenmp -lpthread -lgomp

array:
	@echo "creating array";
	for size in 10 100 1000000 ; do \
		./src/create_array.sh $$size results; \
	done

style : 
	astyle -A1 *.c --recursive
	@if ls *.h >/dev/null 2>&1; then astyle -A1 *.h; fi
	find . -type f -name "*.orig" -delete

clean :
	rm -rf bin
