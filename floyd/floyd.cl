
__kernel void floyd(int elements, __global int *graph,
                    __global int *output_graph, int k) {
  int i = get_global_id(0); // Row index
  int j = get_global_id(1); // Column index

  // Only process valid indices
  if (i < elements && j < elements) {
    int current = graph[i * elements + j];
    int via_k = graph[i * elements + k] + graph[k * elements + j];

    // Update the output matrix only if a shorter path is found
    output_graph[i * elements + j] = (current > via_k) ? via_k : current;
  }
}