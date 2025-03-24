#include <stdio.h>
#include <gmp.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <time.h>
#include "gmp_redblack.h"
#include <omp.h>
#include <stdint.h>
#include <inttypes.h>

//________Ska va helt ärlig jag fattar inte vad som händer riktigt men det funkar___________
typedef unsigned __int128 __uint128_t;

#define P10_UINT64 10000000000000000000ULL   /* 19 zeroes */
#define E10_UINT64 19

#define STRINGIZER(x)   # x
#define TO_STRING(x)    STRINGIZER(x)

static int print_u128_u(__uint128_t u128)
{
    int rc;
    if (u128 > UINT64_MAX)
    {
        __uint128_t leading  = u128 / P10_UINT64;
        uint64_t  trailing = u128 % P10_UINT64;
        rc = print_u128_u(leading);
        rc += printf("%." TO_STRING(E10_UINT64) PRIu64, trailing);
    }
    else
    {
        uint64_t u64 = u128;
        rc = printf("%" PRIu64, u64);
    }
    return rc;
}
static void uint128_to_string(__uint128_t u128, char *buffer, size_t buffer_size) {
    if (u128 > UINT64_MAX) {
        __uint128_t leading = u128 / P10_UINT64;
        uint64_t trailing = u128 % P10_UINT64;

        // Recursively process the higher part
        uint128_to_string(leading, buffer, buffer_size);

        // Append the lower part with zero-padding
        size_t len = strlen(buffer);
        snprintf(buffer + len, buffer_size - len, "%." TO_STRING(E10_UINT64) PRIu64, trailing);
    } else {
        // Base case: number fits within uint64_t
        uint64_t u64 = u128;
        snprintf(buffer, buffer_size, "%" PRIu64, u64);
    }
}
void write_uint128_to_file(const char *filename, __uint128_t number) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Error opening file for writing");
        fprintf(stderr, "Failed to open file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    // Convert the 128-bit number to a string
    char buffer[40]; // Buffer to hold the string representation
    uint128_to_string(number, buffer, sizeof(buffer));

    // Write the string to the file
    fprintf(file, "%s\n", buffer);

    fclose(file);
}
// _____________________________________


#define CURRENT_NUMBER_FILE "currentNumber.txt"
#define SUS_NUMBER_FILE "sussyNumbers.txt"

#define DEFAULT_ITERATIONS 150
#define DEFAULT_TREE_SIZE 1000000 //1 million => ca 76MB (mpz > 2^68)
#define MAX_SAVE_SEQUENCE_LENGTH 10000
#define DEFAULT_THREAD_COUNT 1

// Function prototypes
void verifyFileExistence(const char *filename);
void updateFile(const char *filename, mpz_t number, int base, const char *mode);
void collatz_sequence(rb_tree *tree, mpz_t start_number, int *tree_size, int max_tree_size, int max_iterations);


void write_uint128_to_file(const char *filename, __uint128_t number);
__uint128_t read_uint128_from_file(const char *filename);
void uint128_to_mpz(mpz_t mpz, __uint128_t num);

void print_uint128(__uint128_t num);

int main(int argc, char **argv) {
    // Argument parsing
    int max_iterations = DEFAULT_ITERATIONS;
    int max_tree_size = DEFAULT_TREE_SIZE;
    int num_threads = DEFAULT_THREAD_COUNT;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-iterations") == 0 && i + 1 < argc) {
            max_iterations = atoi(argv[++i]);
            if (max_iterations <= 0) {
                printf("Invalid value for iterations, using default: %d\n", DEFAULT_ITERATIONS);
                max_iterations = DEFAULT_ITERATIONS;
            }
        }

        else if (strcmp(argv[i], "-treesize") == 0 && i + 1 < argc) {
            max_tree_size = atoi(argv[++i]);
            if (max_tree_size <= 0) {
                printf("Invalid value for max tree size, using default: %d\n", DEFAULT_TREE_SIZE);
                max_tree_size = DEFAULT_TREE_SIZE;
            }
        }
        else if (strcmp(argv[i], "-threads") == 0 && i + 1 < argc) {
            num_threads = atoi(argv[++i]);
            if (num_threads <= 0) {
                printf("Invalid value for number of threads, suing defauly: %d\n", DEFAULT_THREAD_COUNT);
                num_threads = DEFAULT_THREAD_COUNT;
            }
        }
        else {
            printf("Invalid argument: %s\n", argv[i]);
            break;
        }
    }
    printf("_____________________________________\n");
    printf("max_iterations: %d\n", max_iterations);
    printf("max_tree_size: %d\n", max_tree_size);
    printf("num of threads: %d\n", num_threads);
    printf("_____________________________________\n");
    
    // setup
    int terminate = 0;
    verifyFileExistence(CURRENT_NUMBER_FILE);
    verifyFileExistence(SUS_NUMBER_FILE);

    //trees
    mpz_t mpz_number;
    mpz_init(mpz_number);
    rb_tree trees[num_threads];
    int tree_sizes[num_threads];
    for (int i = 0; i < num_threads; i++) {
        initializeTree(&trees[i], mpz_number);
        tree_sizes[i] = 1;
    }

    __uint128_t start_number = read_uint128_from_file("currentNumber.txt");
    __uint128_t chunk_size = 10000000;
    __uint128_t goalpost = start_number + chunk_size;

    

    //collatz conjecture
    while (!terminate) {
        
        #pragma omp parallel num_threads(num_threads)
        {
            int thread_id = omp_get_thread_num();
            mpz_t current_number;
            mpz_init(current_number);
            
            #pragma omp for schedule(dynamic)
            for (uint64_t i = 0; i < (goalpost - start_number); i++) {
                __uint128_t j = start_number + i;
                uint128_to_mpz(current_number, j);
                collatz_sequence(&trees[thread_id], current_number, &tree_sizes[thread_id], max_tree_size, max_iterations);
            }
            mpz_clear(current_number);
        }
        
        start_number += chunk_size;
        goalpost += chunk_size;

        #pragma omp critical
        {
            write_uint128_to_file(CURRENT_NUMBER_FILE, start_number);
        }

        if (kbhit()) {
            terminate = 1;
        }
    }

    mpz_clear(mpz_number);

    printf("Cleaning up trees...\n");
    for (int i = 0; i < num_threads; i++) {
        cleanupTree(&trees[i]);
    }
    printf("Trees removed\n");
    
    return 0;
}

void collatz_sequence(rb_tree *tree, mpz_t start_number, int *tree_size, int max_tree_size, int max_iterations) {
    mpz_t current_number;
    mpz_init(current_number);
    mpz_set(current_number, start_number);
    mpz_t sequence[MAX_SAVE_SEQUENCE_LENGTH];
    int sequence_index = 0;

    int iterations = 0;
    //gmp_printf ("%Zd on thread %d\n", current_number, omp_get_thread_num());

    while( mpz_cmp_d(current_number, 1) > 0 && mpz_cmp(current_number, start_number) >= 0 && searchNode(tree, current_number) == NULL && iterations < max_iterations) {
        //sequence saving
        if (sequence_index < MAX_SAVE_SEQUENCE_LENGTH) {
            mpz_init(sequence[sequence_index]);
            mpz_set(sequence[sequence_index], current_number);
            sequence_index++;
        }
        
        if (mpz_odd_p(current_number)) {
            mpz_mul_ui(current_number, current_number, 3);// * 3
            mpz_add_ui(current_number, current_number, 1);// + 1
            mpz_fdiv_q_2exp(current_number, current_number, 1); // /2

        } else {
            mpz_fdiv_q_2exp(current_number, current_number, 1); // /2
        }
        iterations++;
    }
    // delete unecessary numbers
    while (findMinimumNode(tree) != NULL && mpz_cmp(findMinimumNode(tree)->value, start_number) <= 0) {
        deleteMinimum(tree);
    }
    // save sequence to redblack tree
    for (int i = sequence_index - 1; i >= 0; i--) {
        if (searchNode(tree, sequence[i]) == NULL) {
            if (*tree_size < max_tree_size) {
                insertNode(tree, sequence[i]);
                (*tree_size)++;
            }
        }
        mpz_clear(sequence[i]);
    }
    mpz_clear(current_number);

    // save if sus number
    if (iterations >= max_iterations) {
        updateFile(SUS_NUMBER_FILE, start_number, 10, "a");
    }
}

// convert for initialized to a mpz for collatz_sequence
void uint128_to_mpz(mpz_t mpz, __uint128_t num) {
    uint64_t high = (uint64_t)(num >> 64);
    uint64_t low = (uint64_t)(num & 0xFFFFFFFFFFFFFFFFULL);
    mpz_set_ui(mpz, high);
    mpz_mul_2exp(mpz, mpz, 64);
    mpz_add_ui(mpz, mpz, low);
}

// mpz_t sus number file
void updateFile(const char *filename, mpz_t number, int base, const char *mode) {

    FILE *file = fopen(filename, mode);
    if (file == NULL) {
        printf("Error opening file: %s ", filename);
        return;
    }

    mpz_out_str(file, 10, number);
    fprintf(file, "\n");
    
    fclose(file);
}

void verifyFileExistence(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        file = fopen(filename, "w");
        if (file == NULL) {
            printf("Error creating file: %s\n", filename);
            return;
        }
        fclose(file);
    } else {
        fclose(file);
    }
}

__uint128_t read_uint128_from_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file for reading");
        exit(EXIT_FAILURE);
    }

    char buffer[40];
    if (!fgets(buffer, sizeof(buffer), file)) {
        perror("Error reading from file");
        fclose(file);
        exit(EXIT_FAILURE);
    }
    fclose(file);

    // Validate the input
    for (char *ptr = buffer; *ptr; ptr++) {
        if (*ptr == '\n') break; // Stop at newline
        if (*ptr < '0' || *ptr > '9') {
            fprintf(stderr, "Invalid number in file: %s\n", buffer);
            exit(EXIT_FAILURE);
        }
    }

    // Convert string to __uint128_t
    __uint128_t number = 0;
    char *ptr = buffer;
    while (*ptr >= '0' && *ptr <= '9') {
        number = number * 10 + (*ptr - '0');
        ptr++;
    }

    return number;
}
