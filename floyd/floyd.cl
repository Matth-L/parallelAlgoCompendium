__kernel void floyd(int elements, __global int *graph,
                    __global int *output_graph) {

  int i = get_global_id(0);
  int j = get_global_id(1);

  output_graph[i * elements + j] = graph[i * elements + j];

  for (int k = 0; k < elements; k++) {
    if (output_graph[i * elements + j] >
        output_graph[i * elements + k] + output_graph[k * elements + j]) {
      output_graph[i * elements + j] =
          output_graph[i * elements + k] + output_graph[k * elements + j];
    }
  }
}
