all : 
	make dir 
	make compile
	make array
	

dir : 
	mkdir -p bin
	mkdir -p results

compile: 
	gcc ./src/sequential_merge_sort.c -o ./bin/sequential_merge_sort -lm -g -lgomp
	gcc ./src/pthread_merge_sort.c -o ./bin/pthread_merge_sort -lm -lpthread -g -lgomp
	gcc ./src/openMP_merge_sort.c -o ./bin/openMP_merge_sort -lm -fopenmp -g -lgomp

array:
	@echo "creating array";
	for size in 10 100  ; do \
		./src/create_array.sh $$size results; \
	done

style : 
	astyle -A1 *.c --recursive
	@if ls *.h >/dev/null 2>&1; then astyle -A1 *.h; fi
	find . -type f -name "*.orig" -delete

benchmark :
	make style
	make dir
	make array
	make compile

###############################################################################

# running sequential merge sort

	@echo "Running sequential merge sort";
	for size in 1000000 100000000; do \
		./bin/sequential_merge_sort ./results/unsorted_array_$$size.txt; \
	done

################################################################################

	# for threads in 1 2 4 8 16 24 48; do \
	# 	echo "Number of threads is $$threads"; \
	# 	echo "Running pthread_merge_sort"; \
	# 	./bin/pthread_merge_sort $$threads; \
	# 	echo "Running openMP_merge_sort"; \
	# 	./bin/openMP_merge_sort $$threads; \
	# done	

###############################################################################

	@echo "Benchmarking done"
clean :  
	rm -rvf bin
	rm -rvf results
	rm -rvf *.txt

