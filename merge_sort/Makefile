all : 
	make dir
	make test

dir : 
	mkdir -p bin
	mkdir -p results

compile:
	make dir
	@echo "Compiling"
	gcc  ./src/d2s.c -fopenmp -o ./bin/d2s
	gcc  ./src/d2p.c -lpthread -fopenmp -o ./bin/d2p
	gcc  ./src/d2omp.c -fopenmp -o ./bin/d2omp

test:
	make dir
	@echo "Running tests"
	touch resultat_seq resultat_pth resultat_omp
	@echo "Test sequential"
	gcc ./src/d2s.c -fopenmp
	./a.out ./results/unsorted_array_100000.txt ./resultat_seq
	@echo "Test pthread with 16 threads"
	gcc ./src/d2p.c -lpthread -fopenmp
	./a.out 16 ./results/unsorted_array_100000.txt ./resultat_pth
	@echo "Test openMP with 16 threads"
	gcc ./src/d2omp.c -fopenmp
	./a.out 16 ./results/unsorted_array_100000.txt ./resultat_omp

array:
	@echo "creating array";
	for size in 10 10000 1000000 ; do \
		./src/create_array.sh $$size results; \
	done

benchmark:
	make dir
	@echo "Running benchmark"
	gcc src/benchmark.c -fopenmp -lpthread -o ./bin/benchmark
	./bin/benchmark ./results/unsorted_array_100000.txt 

clean:
	rm -fv a.out
	rm -fv ./bin/*
	rm -fv resultat_*
	rm -rf bin
