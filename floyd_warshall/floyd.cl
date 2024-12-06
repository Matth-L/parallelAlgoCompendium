__kernel void floyd(const int elements, __global int *graph,
                    __global int *output_graph, const int k) {
  int i = get_global_id(0); // Row index
  int j = get_global_id(1); // Column index

  if (i < elements && j < elements) {

    int current = graph[i * elements + j];
    int other_path = graph[i * elements + k] + graph[k * elements + j];

    if (current > other_path) {
      output_graph[i * elements + j] = other_path;
    } else {
      output_graph[i * elements + j] = current;
    }
  }
}