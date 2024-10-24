all : 
	mkdir -p bin
	./src/create_array.sh 16
	gcc src/sequential_merge_sort.c -o bin/sequential_merge_sort -g 
	gcc src/pthread_merge_sort.c -o bin/pthread_merge_sort -lpthread -g -lm

style : 
	astyle -A1 *.c --recursive
	@if ls *.h >/dev/null 2>&1; then astyle -A1 *.h; fi
	find . -type f -name "*.orig" -delete

clean :  
	rm -rvf bin
	rm -rvf *.txt

