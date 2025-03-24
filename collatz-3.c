#include <stdio.h>
#include <gmp.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <time.h>

#define CURRENT_NUMBER_FILE "currentNumber.txt"
#define SUS_NUMBER_FILE "sussyNumbers.txt"

#define DEFAULT_ITERATIONS 150
#define DEFAULT_GOALPOST 100

// Declare GMP integer globally for cleanup
mpz_t start_number, current_number;

// Function prototypes
void cleanup();
void updateFile(const char *filename, mpz_t number, int base, const char *mode);
void verifyFileExistence(const char *filename);
void readCurrentNumberFile(const char *filename, mpz_t number, int base);


int main(int argc, char **argv) {
    // Argument parsing
    int max_iterations = DEFAULT_ITERATIONS;
    int goalpost = DEFAULT_GOALPOST;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-iterations") == 0 && i + 1 < argc) {
            max_iterations = atoi(argv[++i]);
            if (max_iterations <= 0) {
                printf("Invalid value for iterations, using default: %d\n", DEFAULT_ITERATIONS);
                max_iterations = DEFAULT_ITERATIONS;
            }
        }

        else if (strcmp(argv[i], "-goalpost") == 0 && i + 1 < argc) {
            goalpost = atoi(argv[++i]);
            if (goalpost <= 0) {
                printf("Invalid value for goalpost, using default: %d\n", DEFAULT_GOALPOST);
                goalpost = DEFAULT_GOALPOST;
            }
        }
        else {
            printf("Invalid argument: %s\n", argv[i]);
            break;
        }
    }
    printf("max_iterations: %d\n", max_iterations);
    printf("goalpost: %d\n", goalpost);
    

    // setup
    mpz_init(start_number);
    mpz_init(current_number);
    volatile int iteration = 0;
    int terminate = 0;
    
    verifyFileExistence(CURRENT_NUMBER_FILE);
    verifyFileExistence(SUS_NUMBER_FILE);
    readCurrentNumberFile(CURRENT_NUMBER_FILE, start_number, 10);

    //timer
    clock_t t = clock();

    //collatz conjecture
    while (!terminate) {
        mpz_set(current_number, start_number);
        while( mpz_cmp_d(current_number, 1) > 0 && mpz_cmp(current_number, start_number) >= 0) {
            if (mpz_odd_p(current_number)) {

                mpz_mul_ui(current_number, current_number, 3);// * 3
                mpz_add_ui(current_number, current_number, 1);// + 1
                mpz_fdiv_q_2exp(current_number, current_number, 1); // /2
            } else {
                mpz_fdiv_q_2exp(current_number, current_number, 1); // /2
            }

        }
        mpz_add_ui(start_number, start_number, 1); // +1
        
        if (kbhit() || mpz_cmp_d(start_number, goalpost) > 0) {
            terminate = 1;
        }
        
    }

    t = clock() - t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC;
    printf("Time for algorithm to reach goalpost: %f seconds\n", time_taken);

    cleanup();
    
    


    return 0;
}

// Cleanup function to clear memory utifall att
void cleanup() {
    printf("Most recent start_number saving...\n");
    mpz_sub_ui(start_number, start_number, 1);
    updateFile(CURRENT_NUMBER_FILE, start_number, 10, "w");
    printf("Most recent start_number saved\n");

    printf("Freeing memory...\n");
    mpz_clear(start_number);
    mpz_clear(current_number);
    printf("Memory freed\n");
}

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

void readCurrentNumberFile(const char *filename, mpz_t number, int base){
    FILE *file = fopen(filename, "r");
    if (file == NULL){
        printf("Error opening file: %s", filename);
        return;
    }
    mpz_inp_str(number, file, 10);
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