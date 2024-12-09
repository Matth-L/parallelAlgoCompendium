# ParallelAlgoCompendium 📚

This repository showcases three projects aimed at parallelizing algorithms using Pthreads/OpenMP, MPI, and OpenCL.

## Merge Sort (Pthreads/OpenMP)

The focus is on implementing and analyzing the performance of recursive parallel and sequential merge sort algorithms. The parallel version uses the pthread library and OpenMP. Both implementations are written in C, with performance evaluated across different thread counts.

<p align="center">
    <img src="./img/merge_sort.gif" alt="Merge Sort" width="400"/>
</p>

To test, execute the following command:

```bash
cd merge_sort
make
```

## Sexy Number (MPI) 

The goal is to parallelize the Sieve of Eratosthenes to find sexy numbers, optimizing workload distribution to minimize memory usage with MPI.

<p align="center">
    <img src="./img/sexy_number.gif" alt="Sexy Number" width="400"/>
</p>

To test, execute the following command:

```bash
cd sexy_number
make p1 #we only used MPI_send, MPI_get
make p2 #we replaced them with MPI_Put and MPI_send
```

## Floyd-Warshall (OpenCL) 

Finally, we had to parallelize the Floyd-Warshall algorithm using OpenCL. This algorithm finds the shortest path between all pairs of vertices in a weighted graph.

<p align="center">
    <img src="./img/floyd_warshall.gif" alt="Floyd-Warshall" width="400"/>
</p>   

To test, execute the following command:

```bash
cd floyd_warshall
make
./main <number_of_nodes>
```

You can also plot with this command : 
```bash
make plot
```

This code is adapted for my GPU 0, so you may need to change the device type and device number in the code.


(images are from wikipedia)
