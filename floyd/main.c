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

#define INF 1e9

/**********************************************
 * @brief build the graph using the following
 * instruction :
 *
 * "
 * Il y a n noeuds numerotees de 0 a n − 1;
 * Pour tout 0 ≤ i ≤ n − 1 l’arc de i a i vaut 0;
 * Pour tout noeud i < n − 1, il y a un arc de i a i + 1 de longueur 2;
 * Il y a un arc de n − 1 `a 0 de longueur 5;
 * Tous les autres arcs valent 5n.
 * "
 *
 * @param n the number of nodes
 * @return int** the matrix that represents the graph
 ***********************************************/
void init_graph(int n, int **graph)
{
    *graph = (int *)malloc(n * n * sizeof(int));
    if (*graph == NULL)
    {
        printf("Error allocating memory\n");
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
 * @brief it's the graph from this video :
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
        printf("Error allocating memory\n");
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
 *
 * @param graph
 * @param n
 ***********************************************/
void print_graph(int *graph, int n)
{
    for (int i = 0; i < n; i++)
    {
        printf("Node %d: ", i);
        for (int j = 0; j < n; j++)
        {
            printf("%d ", graph[i * n + j]);
        }
        printf("\n");
    }
}

void floydWarshall(int *graph, int n)
{
    int i, j, k;

    /* Add all vertices one by one to the set of intermediate vertices.
      ---> Before start of a iteration, we have shortest distances between all
      pairs of vertices such that the shortest distances consider only the
      vertices in set {0, 1, 2, .. k-1} as intermediate vertices.
      ----> After the end of a iteration, vertex no. k is added to the set of
      intermediate vertices and the set becomes {0, 1, 2, .. k} */
    for (k = 0; k < n; k++)
    {
        // Pick all vertices as source one by one
        for (i = 0; i < n; i++)
        {
            // Pick all vertices as destination for the
            // above picked source
            for (j = 0; j < n; j++)
            {
                // If vertex k is on the shortest path from
                // i to j, then update the value of graph[i * n + j]
                if (graph[i * n + k] + graph[k * n + j] < graph[i * n + j])
                    graph[i * n + j] = graph[i * n + k] + graph[k * n + j];
            }
        }
    }
}

/**********************************************
 * @brief load the program source
 *
 * @param filename
 * @return char*
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
        printf("Echec\n");
        return 0;
    }
    if (stat(filename, &status) == 0)
        sz = (int)status.st_size;
    source = (char *)malloc(sz + 1);
    fread(source, sz, 1, fp);
    source[sz] = '\0';
    return source;
}

int main(int argc, char **argv)
{
    // initialize the graph
    int elements = 1000;

    int *graph = NULL;

    // init output_graph
    int *output_graph = malloc(elements * elements * sizeof(int *));

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

    if (graph == NULL)
    {
        printf("Error allocating memory\n");
        exit(EXIT_FAILURE);
    }

    printf("graph before floyd\n");
    print_graph(graph, elements);

    // Load the OpenCL code
    char *programSource = load_program_source("./floyd.cl");

    // Compute the size of the data
    size_t datasize = sizeof(int) * elements * elements;

    cl_int status;

    //-----------------------------------------------------
    // STEP 1: Discover and initialize the platforms
    //-----------------------------------------------------
    cl_uint numPlatforms = 0;
    cl_platform_id *platforms = NULL;

    // Calcul du nombre de plateformes
    status = clGetPlatformIDs(0, NULL, &numPlatforms);
    if (status)
    {
        printf("ERREUR A LA RECUPERATION DU NOMBRE DE PLATEFORMES: %d\n",
               status);
    }

    printf("Number of platforms = %d\n", numPlatforms);

    // Allocation de l'espace
    platforms = (cl_platform_id *)malloc(numPlatforms * sizeof(cl_platform_id));

    // Trouver les plateformes
    status = clGetPlatformIDs(numPlatforms, platforms, NULL);

    if (status)
    {
        printf("ERREUR A LA RECUPERATION DES PLATEFORMES: %d\n", status);
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

    // calcul du nombre de périphériques
    status = clGetDeviceIDs(platforms[0],
                            CL_DEVICE_TYPE_ALL,
                            0,
                            NULL,
                            &numDevices);

    if (status)
    {
        printf("ERREUR A LA RECUPERATION DU NOMBRE DE PERIPHERIQUES: %d\n",
               status);
    }
    printf("Number of devices = %d\n", (int)numDevices);

    // Allocation de l'espace
    devices = (cl_device_id *)malloc(numDevices * sizeof(cl_device_id));

    // Trouver les périphériques
    status = clGetDeviceIDs(platforms[0],
                            CL_DEVICE_TYPE_ALL,
                            numDevices,
                            devices,
                            NULL);

    if (status)
    {
        printf("ERREUR A LA RECUPERATION DES PERIPHERIQUES: %d\n", status);
    }

    for (int i = 0; i < (int)numDevices; i++)
    {
        clGetDeviceInfo(devices[i], CL_DEVICE_NAME, sizeof(Name), Name, NULL);
        printf("Name of device %d: %s\n\n", i, Name);
    }

    //-----------------------------------------------------
    // STEP 3: Create a context
    //-----------------------------------------------------
    printf("Création du contexte\n");
    fflush(stdout);
    cl_context context = NULL;
    context = clCreateContext(NULL, 1, &devices[0], NULL, NULL, &status);

    if (status)
    {
        printf("ERREUR A LA CREATION DU CONTEXTE: %d\n", status);
    }

    //-----------------------------------------------------
    // STEP 4: Create a command queue
    //-----------------------------------------------------
    printf("Création de la file d'attente\n");
    fflush(stdout);
    cl_command_queue cmdQueue;
    cmdQueue = clCreateCommandQueue(context, devices[0], 0, &status);

    if (status)
    {
        printf("ERREUR A LA CREATION DE LA FILE D'ATTENTE: %d\n", status);
    }

    //-----------------------------------------------------
    // STEP 5: Create device buffers
    //-----------------------------------------------------
    printf("Création des buffers\n");
    fflush(stdout);
    cl_mem bufferGraph;
    cl_mem output_buffer_graph;
    bufferGraph = clCreateBuffer(context,
                                 CL_MEM_READ_WRITE,
                                 datasize,
                                 NULL,
                                 &status);
    if (status)
    {
        printf("ERREUR A LA CREATION DU BUFFER bufferGraph: \n");
    }
    output_buffer_graph = clCreateBuffer(context,
                                         CL_MEM_WRITE_ONLY,
                                         datasize,
                                         NULL,
                                         &status);

    if (status)
    {
        printf("ERREUR A LA CREATION DU BUFFER output_buffer_graph \n");
    }
    //-----------------------------------------------------
    // STEP 6: Write host data to device buffers
    //-----------------------------------------------------
    printf("Ecriture dans les buffers\n");
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

    if (status)
    {
        printf("ERREUR A L'ECRITURE: %d\n", status);
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

    if (status)
    {
        printf("ERREUR A LA CREATION DU PROGRAMME: %d\n", status);
    }

    printf("Compilation\n");
    fflush(stdout);

    status = clBuildProgram(program, 1, &devices[0], NULL, NULL, NULL);
    if (status)
    {
        printf("ERREUR A LA COMPILATION: %d\n", status);
    }

    //-----------------------------------------------------
    // STEP 8: Create the kernel
    //-----------------------------------------------------
    cl_kernel kernel = NULL;

    printf("Création du kernel\n");
    fflush(stdout);

    kernel = clCreateKernel(program, "floyd", &status);
    if (status)
    {
        printf("ERREUR A LA CREATION DU KERNEL: %d\n", status);
    }

    //-----------------------------------------------------
    // STEP 9: Set the kernel arguments
    //-----------------------------------------------------
    // Associate the input and output buffers with the
    // kernel using clSetKernelArg()
    printf("Passage des paramètres\n");
    fflush(stdout);

    for (int k = 0; k < elements; k++)
    {
        clSetKernelArg(kernel, 0, sizeof(cl_int),
                       (void *)&elements);
        clSetKernelArg(kernel, 1, sizeof(cl_mem),
                       (void *)&bufferGraph);
        clSetKernelArg(kernel, 2, sizeof(cl_mem),
                       (void *)&output_buffer_graph);
        clSetKernelArg(kernel, 3, sizeof(cl_int),
                       (void *)&k);
        size_t global_work_size[] = {elements, elements};
        clEnqueueNDRangeKernel(cmdQueue, kernel, 2, NULL, global_work_size, NULL, 0, NULL, NULL);

        clEnqueueCopyBuffer(cmdQueue, output_buffer_graph, bufferGraph, 0, 0, sizeof(int) * elements * elements, 0, NULL, NULL);
    }

    //-----------------------------------------------------
    // STEP 12: Read the output buffer back to the host
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

    if (output_buffer_graph == NULL)
    {
        printf("output_buffer_graph is NULL\n");
    }

    printf("graph after floyd\n");
    print_graph(output_graph, elements);

    floydWarshall(graph, elements);
    printf("graph after floyd github\n");
    print_graph(graph, elements);

    int error_count = 0;

    for (int i = 0; i < elements; i++)
    {
        for (int j = 0; j < elements; j++)
        {
            if (graph[i * elements + j] != output_graph[i * elements + j])
            {
                error_count++;
            }
        }
    }

    printf("Matrix size : %d*%d, Errors  %d\n", elements, elements, error_count);

    //-----------------------------------------------------
    // STEP 13: Release OpenCL resources
    //-----------------------------------------------------
    // Free OpenCL resources
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(cmdQueue);
    clReleaseMemObject(bufferGraph);
    clReleaseContext(context);

    // Free host resources
    free(graph);
    free(platforms);
    free(devices);

    return 0;
}
