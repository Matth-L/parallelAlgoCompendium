all : 
	mkdir -p bin
	./src/create_array.sh 16
	gcc src/sequential_merge_sort.c -o bin/sequential_merge_sort -g 
	gcc src/pthread_merge_sort.c -o bin/pthread_merge_sort -lpthread -g -lm

style : 
	astyle -A1 *.c --recursive
	@if ls *.h >/dev/null 2>&1; then astyle -A1 *.h; fi
	find . -type f -name "*.orig" -delete

benchmark :

###############################################################################
	mkdir -p bin

###############################################################################

	@echo "creating array";
	for size in 10 100 1000 10000 100000 10000000; do \
		./src/create_array.sh $$size; \
	done

###############################################################################

	@echo "Running sequential_merge_sort"; \
	./bin/sequential_merge_sort unsorted_array_100; \

################################################################################

	for threads in 1 2 4 8 16 24 48; do \
		echo "Number of threads is $$threads"; \
		echo "Running pthread_merge_sort"; \
		./bin/pthread_merge_sort $$threads; \
		echo "Running openMP_merge_sort"; \
		./bin/openMP_merge_sort $$threads; \
	done

###############################################################################

	@echo "Benchmarking done"
clean :  
	rm -rvf bin
	rm -rvf *.txt

