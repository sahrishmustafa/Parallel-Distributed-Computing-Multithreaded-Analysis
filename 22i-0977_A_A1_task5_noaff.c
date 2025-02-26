#define _POSIX_C_SOURCE 199309L // to use clock monotonic
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>
#include <time.h>

#define ROW_TOTAL 35040
#define COLUMN_TOTAL 8600
#define MAX_TID 12  

float matrix[ROW_TOTAL][COLUMN_TOTAL];  
long long total_sum = 0;  // Shared sum
int global_max = INT_MIN; // Shared max
int global_min = INT_MAX; // Shared min
long long row_sums[ROW_TOTAL];  // Row-wise sums
long long col_sums[COLUMN_TOTAL];  // Column-wise sums

pthread_mutex_t sum_mutex, min_max_mutex, col_mutex;  // Mutexes

typedef struct {
    int start_row;
    int end_row;
} ThreadData;

//compute sum, min/max, row-wise, and column-wise sums
void* sum_matrix(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    long long local_sum = 0;
    int local_max = INT_MIN;
    int local_min = INT_MAX;
    long long local_col_sums[COLUMN_TOTAL] = {0}; 
    // all local variables which are independent for each thread

    for (int i = data->start_row; i < data->end_row; i++) {
        row_sums[i] = 0;  
        for (int j = 0; j < COLUMN_TOTAL; j++) {
            int num = matrix[i][j];
            // one number read

            // local sums
            local_sum += num;
            row_sums[i] += num;  
            local_col_sums[j] += num;  

            if (num > local_max){
                local_max = num;
            }
            if (num < local_min){
                local_min = num;
            }
        }
    }

    // global access to shared variables under mutexes
    pthread_mutex_lock(&sum_mutex);
    total_sum += local_sum;
    pthread_mutex_unlock(&sum_mutex);

    pthread_mutex_lock(&min_max_mutex);
    if (local_max > global_max){
        global_max = local_max;
    } 
    if (local_min < global_min){
        global_min = local_min;
    } 
    pthread_mutex_unlock(&min_max_mutex);

    pthread_mutex_lock(&col_mutex);
    for (int j = 0; j < COLUMN_TOTAL; j++) {
        col_sums[j] += local_col_sums[j]; 
    }
    pthread_mutex_unlock(&col_mutex);

    free(data);
    return NULL;
}

void read_matrix_from_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < ROW_TOTAL; i++) {
        for (int j = 0; j < COLUMN_TOTAL; j++) {
            if (fscanf(file, "%f", &matrix[i][j]) != 1) {  
                fprintf(stderr, "Error reading matrix data at (%d, %d).\n", i, j);
                fclose(file);
                exit(EXIT_FAILURE);
            }
        }
    }

    fclose(file);
}

// Function for Output verification
void write_results_to_file(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "Total sum of matrix elements: %lld\n", total_sum);
    fprintf(file, "Maximum element in matrix: %d\n", global_max);
    fprintf(file, "Minimum element in matrix: %d\n", global_min);

    fprintf(file, "\nRow-wise sums:\n");
    for (int i = 0; i < ROW_TOTAL; i++) {
        fprintf(file, "Row %d Sum: %lld\n", i, row_sums[i]);
    }

    fprintf(file, "\nColumn-wise sums:\n");
    for (int j = 0; j < COLUMN_TOTAL; j++) {
        fprintf(file, "Column %d Sum: %lld\n", j, col_sums[j]);
    }

    fclose(file);
}

int main() {
    pthread_t TID[MAX_TID];
    int rows_per_thread = ROW_TOTAL / MAX_TID;
    struct timespec start, end;
    double elapsed_time_parallel, elapsed_time_serial;

    // Initialize mutex
    pthread_mutex_init(&sum_mutex, NULL);
    pthread_mutex_init(&min_max_mutex, NULL);
    pthread_mutex_init(&col_mutex, NULL);

    clock_gettime(CLOCK_MONOTONIC, &start);
    // Read matrix from file -> SERIAL ~ 40ish seconds
    read_matrix_from_file("matrix.txt");
    clock_gettime(CLOCK_MONOTONIC, &end);
    // Calculate elapsed time in seconds
    elapsed_time_serial = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;    

    clock_gettime(CLOCK_MONOTONIC, &start);

    // summations -> PARALLEL
    for (int i = 0; i < MAX_TID; i++) {
        ThreadData* data = malloc(sizeof(ThreadData));
        data->start_row = i * rows_per_thread;
        data->end_row = (i == MAX_TID - 1) ? ROW_TOTAL : (i + 1) * rows_per_thread;
        pthread_create(&TID[i], NULL, sum_matrix, data);
    }

    for (int i = 0; i < MAX_TID; i++) {
        pthread_join(TID[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_time_parallel = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    // Destroy mutex
    pthread_mutex_destroy(&sum_mutex);
    pthread_mutex_destroy(&min_max_mutex);
    pthread_mutex_destroy(&col_mutex);

    // Output verification
    write_results_to_file("output_matrix.txt");

    printf("Parallel computation took: %.6f seconds\n", elapsed_time_parallel);
    printf("Serial file-reading took: %.6f seconds\n", elapsed_time_serial);
    printf("Number of threads used: %d\n", MAX_TID);

    return 0;
}