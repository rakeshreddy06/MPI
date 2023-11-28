#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#define NUM_ELEMENTS 50

// Function to compare two integers, used by qsort
static int compare_int(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

// Merges two sorted arrays into a single sorted array
void simple_two_way_merge(int *final_array, int *a, int len_a, int *b, int len_b) {
    int i = 0, j = 0, k = 0;
    // Merge until one array is exhausted
    while (i < len_a && j < len_b) {
        if (a[i] < b[j]) {
            final_array[k++] = a[i++];
        } else {
            final_array[k++] = b[j++];
        }
    }
    // Copy any remaining elements from array a
    while (i < len_a) {
        final_array[k++] = a[i++];
    }
    // Copy any remaining elements from array b
    while (j < len_b) {
        final_array[k++] = b[j++];
    }
}

int main(int argc, char **argv) {
    int rank, size;
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Calculate number of elements per process
    int elements_per_proc = NUM_ELEMENTS / size;
    int remainder = NUM_ELEMENTS % size;
    // Allocate memory for sub-array with extra space for possible remainder
    int *sub_array = malloc((elements_per_proc + 1) * sizeof(int));
    int *data = NULL;
    int *recvcounts = malloc(size * sizeof(int));
    int *displs = malloc(size * sizeof(int));

    if (rank == 0) {
        // Root process initializes and prints unsorted data
        data = malloc(NUM_ELEMENTS * sizeof(int));
        srand(0);
        printf("Unsorted:\t");
        for (int i = 0; i < NUM_ELEMENTS; i++) {
            data[i] = rand() % NUM_ELEMENTS;
            printf("%d ", data[i]);
        }
        printf("\n");

        // Calculate receive counts and displacements for each process
        int offset = 0;
        for (int i = 0; i < size; i++) {
            recvcounts[i] = elements_per_proc + (i < remainder ? 1 : 0);
            displs[i] = offset;
            offset += recvcounts[i];
        }
    }

    // Determine send count for each process
    int sendcount = elements_per_proc + (rank < remainder ? 1 : 0);
    // Scatter the data to all processes
    MPI_Scatterv(data, recvcounts, displs, MPI_INT, sub_array, sendcount, MPI_INT, 0, MPI_COMM_WORLD);
    // Each process sorts its sub-array
    qsort(sub_array, sendcount, sizeof(int), compare_int);
    // Gather the sorted sub-arrays back to the root process
    MPI_Gatherv(sub_array, sendcount, MPI_INT, data, recvcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        // Root process prints the sorted sub-arrays
        for (int i = 0; i < size; i++) {
            printf("Subarray %d: ", i);
            for (int j = 0; j < recvcounts[i]; j++) {
                printf("%d ", data[displs[i] + j]);
            }
            printf("\n");
        }

        // Prepare for merging the sorted sub-arrays
        int **sub_arrays = malloc(size * sizeof(int *));
        for (int i = 0; i < size; i++) {
            sub_arrays[i] = &data[displs[i]];
        }

        // Sequential merging of the sorted sub-arrays
        int *merged_array = malloc(NUM_ELEMENTS * sizeof(int));
        int *temp_array = malloc(NUM_ELEMENTS * sizeof(int));
        memcpy(merged_array, sub_arrays[0], recvcounts[0] * sizeof(int));
        int merged_size = recvcounts[0];

        for (int i = 1; i < size; i++) {
            simple_two_way_merge(temp_array, merged_array, merged_size, sub_arrays[i], recvcounts[i]);
            memcpy(merged_array, temp_array, (merged_size + recvcounts[i]) * sizeof(int));
            merged_size += recvcounts[i];
        }

        // Print the fully sorted array
        printf("Sorted:\t\t");
        for (int i = 0; i < NUM_ELEMENTS; i++) {
            printf("%d ", merged_array[i]);
        }
        printf("\n");

        // Free memory used for merging
        free(merged_array);
        free(temp_array);
        free(sub_arrays);
        free(data);
    }

    // Free memory allocated for sub-array and MPI buffers
    free(sub_array);
    free(recvcounts);
    free(displs);
    MPI_Finalize();
    return 0;
}

