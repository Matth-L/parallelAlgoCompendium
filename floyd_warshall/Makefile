all:
	gcc -Wall -Wextra -o main main.c -lOpenCL -lm
	
clean:
	rm -v main
	rm -v adj_list.csv

plot:
	gcc -Wall -Wextra -o main main.c -lOpenCL -lm
	./main
	python3 plot_graph.py
	make clean

