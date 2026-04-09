#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <errno.h>

typedef struct {
    int *arr;
    size_t size;
    int tid;
} sort_thread_arg_t;

int cmp_int(const void *a, const void *b) {
    int ia = *(int *)a;
    int ib = *(int *)b;
    return (ia > ib) - (ia < ib);
}

void *sort_thread_func(void *arg) {
    sort_thread_arg_t *targ = (sort_thread_arg_t *)arg;
    qsort(targ->arr, targ->size, sizeof(int), cmp_int);
    return NULL;
}

typedef struct {
    int value;
    int part_idx;
    int elem_idx;
} heap_node_t;

void heapify_down(heap_node_t *heap, size_t heap_size, size_t idx) {
    while (1) {
        size_t left = 2 * idx + 1;
        size_t right = 2 * idx + 2;
        size_t smallest = idx;
        if (left < heap_size && heap[left].value < heap[smallest].value)
            smallest = left;
        if (right < heap_size && heap[right].value < heap[smallest].value)
            smallest = right;
        if (smallest == idx)
            break;
        heap_node_t tmp = heap[idx];
        heap[idx] = heap[smallest];
        heap[smallest] = tmp;
        idx = smallest;
    }
}

void build_heap(heap_node_t *heap, size_t heap_size) {
    for (size_t i = heap_size / 2; i > 0; --i)
        heapify_down(heap, heap_size, i - 1);
}

heap_node_t heap_extract_min(heap_node_t *heap, size_t *heap_size) {
    heap_node_t min = heap[0];
    heap[0] = heap[*heap_size - 1];
    (*heap_size)--;
    heapify_down(heap, *heap_size, 0);
    return min;
}

void heap_insert(heap_node_t *heap, size_t *heap_size, heap_node_t node) {
    size_t idx = *heap_size;
    heap[*heap_size] = node;
    (*heap_size)++;
    // Просеивание вверх
    while (idx > 0) {
        size_t parent = (idx - 1) / 2;
        if (heap[parent].value <= heap[idx].value)
            break;
        heap_node_t tmp = heap[parent];
        heap[parent] = heap[idx];
        heap[idx] = tmp;
        idx = parent;
    }
}

void multi_way_merge(int **parts, size_t *sizes, size_t k, int *result, size_t total_size) {
    if (k == 0) return;
    if (k == 1) {
        memcpy(result, parts[0], total_size * sizeof(int));
        return;
    }

    heap_node_t *heap = malloc(k * sizeof(heap_node_t));
    size_t heap_size = 0;

    size_t *current_idx = calloc(k, sizeof(size_t));

    for (size_t i = 0; i < k; ++i) {
        if (sizes[i] > 0) {
            heap_node_t node;
            node.value = parts[i][0];
            node.part_idx = i;
            node.elem_idx = 0;
            heap_insert(heap, &heap_size, node);
        }
    }

    size_t res_pos = 0;
    while (heap_size > 0) {
        heap_node_t min_node = heap_extract_min(heap, &heap_size);
        result[res_pos++] = min_node.value;

        size_t part = min_node.part_idx;
        size_t next_elem = min_node.elem_idx + 1;
        if (next_elem < sizes[part]) {
            heap_node_t new_node;
            new_node.value = parts[part][next_elem];
            new_node.part_idx = part;
            new_node.elem_idx = next_elem;
            heap_insert(heap, &heap_size, new_node);
        }
    }

    free(heap);
    free(current_idx);
}

int *generate_random_array(size_t n, unsigned int seed) {
    int *arr = malloc(n * sizeof(int));
    if (!arr) {
        perror("malloc");
        return NULL;
    }
    srand(seed);
    for (size_t i = 0; i < n; ++i) {
        arr[i] = rand() % 1000000;  // числа в диапазоне [0, 999999]
    }
    return arr;
}

int is_sorted(int *arr, size_t n) {
    for (size_t i = 1; i < n; ++i) {
        if (arr[i-1] > arr[i])
            return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <array_size> <num_threads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    size_t array_size = atol(argv[1]);
    size_t num_threads = atol(argv[2]);

    if (array_size == 0) {
        fprintf(stderr, "Array size must be positive.\n");
        return EXIT_FAILURE;
    }
    if (num_threads == 0) {
        fprintf(stderr, "Number of threads must be positive.\n");
        return EXIT_FAILURE;
    }
    if (num_threads > array_size) {
        fprintf(stderr, "Number of threads (%zu) exceeds array size (%zu). Reducing to %zu.\n",
                num_threads, array_size, array_size);
        num_threads = array_size;
    }

    unsigned int seed = (unsigned int)time(NULL);
    int *original_array = generate_random_array(array_size, seed);
    if (!original_array) {
        return EXIT_FAILURE;
    }

    int *array = malloc(array_size * sizeof(int));
    if (!array) {
        perror("malloc");
        free(original_array);
        return EXIT_FAILURE;
    }
    memcpy(array, original_array, array_size * sizeof(int));

    printf("Array size: %zu, threads: %zu\n", array_size, num_threads);

    size_t *part_sizes = malloc(num_threads * sizeof(size_t));
    size_t *part_offsets = malloc(num_threads * sizeof(size_t));
    size_t base_size = array_size / num_threads;
    size_t remainder = array_size % num_threads;
    size_t current_offset = 0;
    for (size_t i = 0; i < num_threads; ++i) {
        part_sizes[i] = base_size + (i < remainder ? 1 : 0);
        part_offsets[i] = current_offset;
        current_offset += part_sizes[i];
    }

    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    sort_thread_arg_t *thread_args = malloc(num_threads * sizeof(sort_thread_arg_t));

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    for (size_t i = 0; i < num_threads; ++i) {
        thread_args[i].arr = array + part_offsets[i];
        thread_args[i].size = part_sizes[i];
        thread_args[i].tid = i;
        if (pthread_create(&threads[i], NULL, sort_thread_func, &thread_args[i]) != 0) {
            perror("pthread_create");
            for (size_t j = 0; j < i; ++j) {
                pthread_join(threads[j], NULL);
            }
            free(array);
            free(original_array);
            free(part_sizes);
            free(part_offsets);
            free(threads);
            free(thread_args);
            return EXIT_FAILURE;
        }
    }

    for (size_t i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], NULL);
    }

    int **sorted_parts = malloc(num_threads * sizeof(int *));
    for (size_t i = 0; i < num_threads; ++i) {
        sorted_parts[i] = array + part_offsets[i];
    }

    int *sorted_array = malloc(array_size * sizeof(int));
    if (!sorted_array) {
        perror("malloc sorted_array");
        free(array);
        free(original_array);
        free(part_sizes);
        free(part_offsets);
        free(threads);
        free(thread_args);
        free(sorted_parts);
        return EXIT_FAILURE;
    }

    multi_way_merge(sorted_parts, part_sizes, num_threads, sorted_array, array_size);

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double elapsed = (end_time.tv_sec - start_time.tv_sec) +
                     (end_time.tv_nsec - start_time.tv_nsec) / 1e9;

    int sorted_ok = is_sorted(sorted_array, array_size);
    if (!sorted_ok) {
        fprintf(stderr, "ERROR: Resulting array is NOT sorted!\n");
    } else {
        printf("Sorting completed successfully.\n");
    }

    printf("Elapsed time: %.6f seconds\n", elapsed);

    if (array_size <= 40) {
        printf("Sorted array: ");
        for (size_t i = 0; i < array_size; ++i) {
            printf("%d ", sorted_array[i]);
        }
        printf("\n");
    } else {
        printf("First 20 elements: ");
        for (size_t i = 0; i < 20; ++i) {
            printf("%d ", sorted_array[i]);
        }
        printf("\nLast 20 elements: ");
        for (size_t i = array_size - 20; i < array_size; ++i) {
            printf("%d ", sorted_array[i]);
        }
        printf("\n");
    }

    free(sorted_array);
    free(sorted_parts);
    free(thread_args);
    free(threads);
    free(part_offsets);
    free(part_sizes);
    free(array);
    free(original_array);

    return EXIT_SUCCESS;
}