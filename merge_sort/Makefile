all : 
	gcc sequential_merge_sort.c -o sequential-merge_sort 

style : 
	astyle -A1 *.c 
	@if ls *.h >/dev/null 2>&1; then astyle -A1 *.h; fi
	rm -fv *.orig

clean : 
	rm -fv sequential-merge_sort