/**********************************************
 * @file main.c
 * @brief Implementation of the floyd-warshall
 * algorithm using OpenCL
 ***********************************************/

#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <assert.h>
#include <CL/cl.h>
#include <time.h>

#define INF 1E9

/**********************************************
 * @brief build the graph using the following
 * instruction :
 *
 * "
 * There are n nodes numbered from 0 to n - 1;
 * For all 0 ≤ i ≤ n - 1, the arc from i to i is 0;
 * For all nodes i < n - 1, there is an arc from i to i + 1 of length 2;
 * There is an arc from n - 1 to 0 of length 5;
 * All other arcs are 5n.
 * "
 *
 * @param n the number of nodes
 * @return int** the matrix that represents the graph
 ***********************************************/
void init_graph(int n, int **graph)
{
    *graph = malloc(n * n * sizeof(int));
    if (*graph == NULL)
    {
        printf("Error allocating memory in init_graph\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            if (i == j)
            {
                (*graph)[i * n + j] = 0;
            }
            else if (i == n - 1 && j == 0)
            {
                (*graph)[i * n + j] = 5;
            }
            else if (i == j - 1)
            {
                (*graph)[i * n + j] = 2;
            }
            else
            {
                (*graph)[i * n + j] = 5 * n;
            }
        }
    }
}

/**********************************************
 * @brief Build the graph from this video :
 * https://www.youtube.com/watch?v=4OQeCuLYj-4
 *
 * @return int** the matrix that represents the graph
 ***********************************************/
void demo_graph(int **graph)
{
    //    |> 1  >|
    //(4) |      | (-2)
    //    |      |
    //    | (3)  v
    //    2 ---> 3
    //    ^      |
    //(-1)|      | (2)
    //    |-< 4 <|

    *graph = (int *)malloc(4 * 4 * sizeof(int));
    if (*graph == NULL)
    {
        printf("Error allocating memory in demo_graph\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (i == j)
            {
                (*graph)[i * 4 + j] = 0;
            }
            else
            {
                (*graph)[i * 4 + j] = INF;
            }
        }
    }

    (*graph)[0 * 4 + 2] = -2;
    (*graph)[1 * 4 + 0] = 4;
    (*graph)[1 * 4 + 2] = 3;
    (*graph)[2 * 4 + 3] = 2;
    (*graph)[3 * 4 + 1] = -1;
}

/**********************************************
 * @brief print the graph
 * if there is more than 10 nodes, prints
 * the 5 first and the 5 last nodes
 *
 * @param graph
 * @param n
 ***********************************************/
void print_graph(int *graph, int n)
{
    for (int i = 0; i < n; i++)
    {
        if (i == 5 && n > 10)
        {
            printf("...\n");
            i = n - 5;
        }
        printf("Node %d: ", i);
        for (int j = 0; j < n; j++)
        {
            printf("%d ", graph[i * n + j]);
        }
        printf("\n");
    }
}

/**********************************************
 * @brief check if the results between two
 * graphs are the same
 *
 * @param graph1
 * @param graph2
 * @param n
 ***********************************************/
void check_results(int *graph1, int *graph2, int n)
{
    for (int i = 0; i < n * n; i++)
    {
        if (graph1[i] != graph2[i])
        {
            printf("\033[1;31mThe results are incorrect\033[0m\n");
            return;
        }
    }
    printf("\033[0;32mThe results are correct\033[0m\n");
}

/**********************************************
 * @brief the sequential floyd-warshall algorithm
 *
 * Copied and modified from :
 * https://github.com/AyushRai09
 *
 * @param *graph the graph
 * @param n the number of nodes
 ***********************************************/
void floydWarshall(int *graph, int n)
{
    int i, j, k;
    for (k = 0; k < n; k++)
    {
        for (i = 0; i < n; i++)
        {
            for (j = 0; j < n; j++)
            {
                if (graph[i * n + k] + graph[k * n + j] < graph[i * n + j])
                    graph[i * n + j] = graph[i * n + k] + graph[k * n + j];
            }
        }
    }
}

/**********************************************
 * @brief load the program source
 *
 * @param filename the name of the file
 * @return char* the source code
 ***********************************************/
char *load_program_source(const char *filename)
{
    FILE *fp;
    char *source;
    int sz = 0;
    struct stat status;
    fp = fopen(filename, "r");
    if (fp == 0)
    {
        printf("Load_program_source_failure\n");
        return 0;
    }
    if (stat(filename, &status) == 0)
        sz = (int)status.st_size;
    source = (char *)malloc(sz + 1);
    fread(source, sz, 1, fp);
    source[sz] = '\0';
    return source;
}

/**********************************************
 * @brief export the adjacency list to a file
 *
 * print the size first, then the adjacency list
 * @param adj_list
 * @param size
 * @param filename
 ***********************************************/
void export_adj_list(int *adj_list, int size, const char *filename)
{
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        printf("Erreur d'ouverture du fichier\n");
        return;
    }
    fprintf(file, "%d\n", size);
    for (int i = 0; i < size; i++)
    {
        fprintf(file, "%d", adj_list[i]);
        if (i != size - 1)
        {
            fprintf(file, ",");
        }
    }

    fclose(file);
}

int main(int argc, char **argv)
{

    // initialize the graph
    int elements = 0;
    int *graph = NULL;

    // Checking the arguments
    if (argc == 1)
    {
        printf("Defaulting to 4 nodes\n");
        elements = 4;
        demo_graph(&graph);
    }
    else if (argc == 2)
    {
        printf("Using %s nodes\n", argv[1]);
        elements = atoi(argv[1]);
        init_graph(elements, &graph);
    }
    else
    {
        printf("Usage: %s [n]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // The graph should be initialized
    if (graph == NULL)
    {
        printf("Error allocating memory for the graph\n");
        exit(EXIT_FAILURE);
    }

    // Compute the size of the data
    size_t datasize = sizeof(int) * elements * elements;

    // initialize output_graph
    int *output_graph = calloc(elements * elements, sizeof(int));

    // Load the OpenCL code
    char *programSource = load_program_source("./floyd.cl");
    if (programSource == NULL)
    {
        printf("Error loading program source\n");
        exit(EXIT_FAILURE);
    }

    cl_int status;

    //-----------------------------------------------------
    // STEP 1: Discover and initialize the platforms
    //-----------------------------------------------------

    cl_uint numPlatforms = 0;
    cl_platform_id *platforms = NULL;

    // Compute the number of platforms
    status = clGetPlatformIDs(0, NULL, &numPlatforms);
    if (status != CL_SUCCESS)
    {
        printf("Error getting platform IDs: %d\n", status);
        exit(EXIT_FAILURE);
    }
    printf("Number of platforms = %d\n", numPlatforms);

    // Find the platforms
    platforms = (cl_platform_id *)malloc(numPlatforms * sizeof(cl_platform_id));
    status = clGetPlatformIDs(numPlatforms, platforms, NULL);
    if (status != CL_SUCCESS)
    {
        printf("Error retrieving platforms: %d\n", status);
        exit(EXIT_FAILURE);
    }

    char Name[1000];
    clGetPlatformInfo(platforms[0], CL_PLATFORM_NAME, sizeof(Name), Name, NULL);
    printf("Name of platform : %s\n", Name);
    fflush(stdout);

    //-----------------------------------------------------
    // STEP 2: Discover and initialize the devices
    //-----------------------------------------------------

    cl_uint numDevices = 0;
    cl_device_id *devices = NULL;

    // Compute the number of devices
    status = clGetDeviceIDs(platforms[0],
                            CL_DEVICE_TYPE_ALL,
                            0,
                            NULL,
                            &numDevices);
    if (status != CL_SUCCESS)
    {
        printf("Error getting device IDs: %d\n", status);
        exit(EXIT_FAILURE);
    }
    printf("Number of devices = %d\n", (int)numDevices);

    // now that we have the number of devices, we can allocate the space
    devices = (cl_device_id *)malloc(numDevices * sizeof(cl_device_id));
    status = clGetDeviceIDs(platforms[0],
                            CL_DEVICE_TYPE_ALL,
                            numDevices,
                            devices,
                            NULL);
    if (status != CL_SUCCESS)
    {
        printf("Error retrieving devices: %d\n", status);
    }

    for (int i = 0; i < (int)numDevices; i++)
    {
        clGetDeviceInfo(devices[i], CL_DEVICE_NAME, sizeof(Name), Name, NULL);
        printf("Name of device %d: %s\n\n", i, Name);
    }

    //-----------------------------------------------------
    // STEP 3: Create a context
    //-----------------------------------------------------

    printf("Creating context\n");
    fflush(stdout);
    cl_context context = NULL;
    context = clCreateContext(NULL, 1, &devices[0], NULL, NULL, &status);

    if (status != CL_SUCCESS)
    {
        printf("Error creating context: %d\n", status);
    }

    //-----------------------------------------------------
    // STEP 4: Create a command queue
    //-----------------------------------------------------

    printf("Creating command queue\n");
    fflush(stdout);

    cl_command_queue_properties p[] = {CL_QUEUE_PROPERTIES,
                                       CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,
                                       0};
    cl_command_queue cmdQueue;

    cmdQueue = clCreateCommandQueueWithProperties(context,
                                                  devices[0],
                                                  p,
                                                  &status);

    if (status != CL_SUCCESS)
    {
        printf("Error creating command queue: %d\n", status);
    }

    //-----------------------------------------------------
    // STEP 5: Create device buffers
    //-----------------------------------------------------

    printf("Creating buffers\n");
    fflush(stdout);
    cl_mem bufferGraph;
    cl_mem output_buffer_graph;

    // We create a buffer for the graph and the output_graph
    bufferGraph = clCreateBuffer(context,
                                 CL_MEM_READ_WRITE,
                                 datasize,
                                 NULL,
                                 &status);
    if (status != CL_SUCCESS)
    {
        printf("Error creating buffer bufferGraph: \n");
    }
    output_buffer_graph = clCreateBuffer(context,
                                         CL_MEM_WRITE_ONLY,
                                         datasize,
                                         NULL,
                                         &status);

    if (status != CL_SUCCESS)
    {
        printf("Error creating buffer output_buffer_graph \n");
    }
    //-----------------------------------------------------
    // STEP 6: Write host data to device buffers
    //-----------------------------------------------------

    printf("Writing to buffers\n");
    fflush(stdout);

    status = clEnqueueWriteBuffer(cmdQueue,
                                  bufferGraph,
                                  CL_TRUE,
                                  0,
                                  datasize,
                                  graph,
                                  0,
                                  NULL,
                                  NULL);

    if (status != CL_SUCCESS)
    {
        printf("Error writing to buffer: %d\n", status);
    }

    //-----------------------------------------------------
    // STEP 7: Create and compile the program
    //-----------------------------------------------------

    printf("CreateProgramWithSource\n");
    fflush(stdout);
    cl_program program =
        clCreateProgramWithSource(context,
                                  1,
                                  (const char **)&programSource,
                                  NULL,
                                  &status);

    if (status != CL_SUCCESS)
    {
        printf("Error creating program: %d\n", status);
    }

    printf("Compilation\n");
    fflush(stdout);

    status = clBuildProgram(program, 1, &devices[0], NULL, NULL, NULL);
    if (status != CL_SUCCESS)
    {
        printf("Error compiling program: %d\n", status);
    }

    //-----------------------------------------------------
    // STEP 8: Create the kernel
    //-----------------------------------------------------
    cl_kernel kernel = NULL;

    printf("Creating kernel\n");
    fflush(stdout);

    kernel = clCreateKernel(program, "floyd", &status);
    if (status != CL_SUCCESS)
    {
        printf("Error creating kernel: %d\n", status);
    }

    //-----------------------------------------------------
    // STEP 9: Set the kernel arguments
    //-----------------------------------------------------

    double start, end;

    // Associate the input and output buffers with the
    // kernel using clSetKernelArg()
    printf("Setting kernel arguments\n");
    printf("starting timer\n");
    start = clock();
    fflush(stdout);

    size_t global_work_size[] = {elements, elements};

    clSetKernelArg(kernel, 0, sizeof(cl_int), (void *)&elements);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&bufferGraph);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&output_buffer_graph);

    for (int k = 0; k < elements; k++)
    {
        clSetKernelArg(kernel, 3, sizeof(cl_int), (void *)&k);

        cl_event kernel_event, copy_event;
        clEnqueueNDRangeKernel(cmdQueue,
                               kernel,
                               2,
                               NULL,
                               global_work_size,
                               NULL,
                               0,
                               NULL,
                               &kernel_event);

        clEnqueueCopyBuffer(cmdQueue,
                            output_buffer_graph,
                            bufferGraph,
                            0,
                            0,
                            datasize,
                            1,
                            &kernel_event, // wait for the kernel to finish
                            &copy_event);

        clWaitForEvents(1, &copy_event); // wait that the copy is finished
        clReleaseEvent(kernel_event);
        clReleaseEvent(copy_event);
    }

    end = clock();

    //-----------------------------------------------------
    // STEP 10: Read the output buffer back to the host
    //-----------------------------------------------------

    clEnqueueReadBuffer(cmdQueue,
                        output_buffer_graph,
                        CL_TRUE,
                        0,
                        datasize,
                        output_graph,
                        0,
                        NULL,
                        NULL);

    //-----------------------------------------------------
    // STEP 11: Checking the results with a sequential algorithm
    //-----------------------------------------------------

    // if the value is too big, we do not test the results
    if (elements < 1000)
    {
        print_graph(output_graph, elements);
        floydWarshall(graph, elements);
        check_results(graph, output_graph, elements);
        export_adj_list(output_graph, elements * elements, "adj_list.csv");
    }

    printf("Done\n");
    printf("Time: %f\n", (end - start) / CLOCKS_PER_SEC);
    //-----------------------------------------------------
    // STEP 12: Release OpenCL resources
    //-----------------------------------------------------

    // Free OpenCL resources
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(cmdQueue);
    clReleaseMemObject(bufferGraph);
    clReleaseMemObject(output_buffer_graph);
    clReleaseContext(context);

    // Free host resources
    free(graph);
    free(output_graph);
    free(platforms);
    free(devices);

    return 0;
}
