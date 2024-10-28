all : 
	

dir : 
	mkdir -p bin
	mkdir -p results

compile: ./src/benchmark.c
	gcc ./src/sequential_merge_sort.c -o ./bin/sequential_merge_sort -lm -g -lgomp
	gcc ./src/pthread_merge_sort.c -o ./bin/pthread_merge_sort -lm -lpthread -g -lgomp
	gcc ./src/openMP_merge_sort.c -o ./bin/openMP_merge_sort -lm -fopenmp -g -lgomp
	gcc ./src/benchmark.c -o ./bin/benchmark -lm -g -lgomp

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
