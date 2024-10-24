all : 
	gcc sequential_merge_sort.c -o sequential_merge_sort -g 
	gcc pthread_merge_sort.c -o pthread_merge_sort -lpthread -g -lm

style : 
	astyle -A1 *.c 
	@if ls *.h >/dev/null 2>&1; then astyle -A1 *.h; fi
	rm -fv *.orig

clean :  
	rm -fv sequential_merge_sort pthread_merge_sort
