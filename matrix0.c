/********************************************************
 * An example source module to accompany...
 *
 * "Using POSIX Threads: Programming with Pthreads"
 * by Brad Nichols, Dick Buttlar, Jackie Farrell
 * O'Reilly & Associates, Inc.
 * to compile, use -lpthread
 * gcc -o matrix0 matrix0.c -lpthread
 ********************************************************
 * matrix0.c --
 * matrix multiplication: MA * MB = MC
 * where each matrix is 10x10 (ARRAY_SIZE x ARRAY_SIZE)
 *
 * A master thread spawns separate child threads to compute each
 * element in the resulting array. That is, 10x10=100 threads
 *
 * Each of the child threads is passed a pointer to a structure
 * that contains the element indices and pointers to starting
 * and resulting arrays.
 *
 * The master thread joins to each thread, prints the result, and
 * exits.
 *********************************************************
 */

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define ARRAY_SIZE 10000

typedef int matrix_t[ARRAY_SIZE][ARRAY_SIZE];

typedef struct {
    int id;
    int size;
    int start_col;
    int end_col;
    matrix_t *MA;
    matrix_t *MB;
    matrix_t *MC;
    pthread_mutex_t *mutex;
} package_t;

matrix_t MA, MB, MC;
pthread_mutex_t mutex;

void multiply(int size, int start_col, int end_col, matrix_t MA, matrix_t MB, matrix_t MC, pthread_mutex_t *mutex) {
    int row, col, k;

    for (col = start_col; col <= end_col; col++) {
        for (row = 0; row < size; row++) {
            MC[row][col] = 0;
            for (k = 0; k < size; k++) {
                MC[row][col] += MA[row][k] * MB[k][col];
            }

            pthread_mutex_lock(mutex);
            printf("Thread %lu updates MC(%d,%d) set to be %d\n", (unsigned long)pthread_self(), row, col, MC[row][col]);
            pthread_mutex_unlock(mutex);
        }
    }
}

void *mult_worker(void *arg) {
    package_t *p = (package_t *)arg;

    int size = p->size;
    int start_col = p->start_col;
    int end_col = p->end_col;

    printf("Thread %d: Computing columns %d to %d\n", p->id, start_col, end_col);
    multiply(size, start_col, end_col, *(p->MA), *(p->MB), *(p->MC), p->mutex);

    printf("Thread %d: Completed columns %d to %d\n", p->id, start_col, end_col);

    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <N> <M>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int M = atoi(argv[2]);

    if (N < 1 || N > ARRAY_SIZE || M < 1 || M > N || N % M != 0) {
        printf("Invalid input. N must be a multiple of M, and both should be within bounds.\n");
        return 1;
    }

    pthread_t *threads = (pthread_t *)malloc(M * sizeof(pthread_t));
    package_t *packages = (package_t *)malloc(M * sizeof(package_t));

    int col_per_thread = N / M;
    int start_col, end_col, i;

    int row, col;

    // Initialize MA and MB matrices with sample data
    for (row = 0; row < N; row++) {
        for (col = 0; col < N; col++) {
            MA[row][col] = 1;
            MB[row][col] = col + 1;
        }
    }

    // Initialize mutex
    pthread_mutex_init(&mutex, NULL);

    for (i = 0; i < M; i++) {
        start_col = i * col_per_thread;
        end_col = (i + 1) * col_per_thread - 1;

        packages[i].id = i;
        packages[i].size = N;
        packages[i].start_col = start_col;
        packages[i].end_col = end_col;
        packages[i].MA = &MA;
        packages[i].MB = &MB;
        packages[i].MC = &MC;
        packages[i].mutex = &mutex;

        pthread_create(&threads[i], NULL, mult_worker, (void *)&packages[i]);
    }

    for (i = 0; i < M; i++) {
        pthread_join(threads[i], NULL);
    }

    // Print the result matrix MC
    printf("Resulting matrix MC:\n");
    for (row = 0; row < N; row++) {
        for (col = 0; col < N; col++) {
            printf("%5d ", MC[row][col]);
        }
        printf("\n");
    }

    // Execute system commands
    system("date; hostname; who | grep htt190001; ps -eaf; ls -l");
    system("who > week10who.txt");

    free(threads);
    free(packages);
    pthread_mutex_destroy(&mutex);

    return 0;
}
