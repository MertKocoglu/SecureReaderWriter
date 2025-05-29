#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#define MAX_THREADS 9          // Maximum number of real readers/writers
#define OPERATIONS 5           // Number of operations each thread performs

int BUFFER = 0;                // Shared resource

// Password table: stores hash values of real threads
unsigned long password_table[2 * MAX_THREADS];  
int password_count = 0;        // Number of entries in the password table
pthread_mutex_t password_mutex; // Mutex to protect access to password table

// Semaphores for synchronization
sem_t resource_access;         // Semaphore to control access to BUFFER
sem_t read_count_access;       // Semaphore to protect read_count variable
int read_count = 0;            // Number of readers currently accessing BUFFER

// Structure to hold thread information
typedef struct {
    int id;                    // Thread ID (1, 2, 3, ...)
    int is_dummy;              // 1 if dummy thread, 0 if real thread
    char role;                 // 'R' for reader, 'W' for writer
} thread_info_t;

// Generates a hash value for the given pthread ID
unsigned long hash_pthread_t(pthread_t thread_id) {
    return (unsigned long)(uintptr_t)thread_id;
}

// Adds a new password hash to the password table if not already present
void add_password(unsigned long hash) {
    pthread_mutex_lock(&password_mutex);
    
    for(int i = 0; i < password_count; i++) {
        if(password_table[i] == hash) {
            pthread_mutex_unlock(&password_mutex);
            return; 
        }
    }
    if(password_count < 2 * MAX_THREADS) {
        password_table[password_count++] = hash;
    } else {
        fprintf(stderr, "Password table full!\n");
    }
    pthread_mutex_unlock(&password_mutex);
}

// Checks whether the given hash is present in the password table (authorization check)
int is_authorized(unsigned long hash) {
    int found = 0;
    pthread_mutex_lock(&password_mutex);
    for(int i = 0; i < password_count; i++) {
        if(password_table[i] == hash) {
            found = 1;
            break;
        }
    }
    pthread_mutex_unlock(&password_mutex);
    return found;
}

// Prints the table header before each test case output
void print_table_header(int test_number) {
    printf("\n--- Test Case %d ---\n", test_number);
    printf("%-10s %-15s %-10s %-10s %-15s\n",
           "Thread_No", "Hash_Value", "Validity", "Role", "Value read/written");
    printf("---------------------------------------------------------------\n");
}

// Reader thread function
void* reader(void* arg) {
    thread_info_t *info = (thread_info_t*) arg;
    unsigned long hash = hash_pthread_t(pthread_self());

    // If real reader, register password
    if(!info->is_dummy) {
        add_password(hash);
    }

    for(int i = 0; i < OPERATIONS; i++) {
        // For real readers, check authorization before proceeding
        if(!info->is_dummy && !is_authorized(hash)) {
            fprintf(stderr, "Reader Thread %d: Unauthorized access!\n", info->id);
            pthread_exit(NULL);
        }

        // Begin critical section to safely update read_count
        sem_wait(&read_count_access);
        read_count++;
        if(read_count == 1) {          // If first reader, lock the resource
            sem_wait(&resource_access);
        }
        sem_post(&read_count_access);

        // Read the BUFFER value and print info
        printf("%-10d %-15lu %-10s %-10s %-15d\n",
               info->id,
               hash,
               info->is_dummy ? "dummy" : "real",
               "reader",
               BUFFER);

        sleep(1);                      // Simulate processing time

        // Exit critical section for read_count update
        sem_wait(&read_count_access);
        read_count--;
        if(read_count == 0) {          // If last reader, unlock the resource
            sem_post(&resource_access);
        }
        sem_post(&read_count_access);

        sleep(1);                      // Pause before next operation
    }

    pthread_exit(NULL);
}

// Writer thread function
void* writer(void* arg) {
    thread_info_t *info = (thread_info_t*) arg;
    unsigned long hash = hash_pthread_t(pthread_self());

    // If real writer, register password
    if(!info->is_dummy) {
        add_password(hash);
    }

    for(int i = 0; i < OPERATIONS; i++) {
        // For real writers, check authorization before proceeding
        if(!info->is_dummy && !is_authorized(hash)) {
            fprintf(stderr, "Writer Thread %d: Unauthorized access!\n", info->id);
            pthread_exit(NULL);
        }

        // Acquire exclusive access to BUFFER
        sem_wait(&resource_access);

        // Write a random value into BUFFER
        BUFFER = rand() % 10000;

        // Print write info
        printf("%-10d %-15lu %-10s %-10s %-15d\n",
               info->id,
               hash,
               info->is_dummy ? "dummy" : "real",
               "writer",
               BUFFER);

        sleep(1);                    // Simulate processing time

        // Release exclusive access
        sem_post(&resource_access);

        sleep(1);                    // Pause before next operation
    }

    pthread_exit(NULL);
}

int main() {
    srand((unsigned int)time(NULL)); // Seed for random numbers
    pthread_mutex_init(&password_mutex, NULL); // Initialize mutex
    sem_init(&resource_access, 0, 1);          // Initialize semaphores
    sem_init(&read_count_access, 0, 1);

    // Test cases: array of (number of readers, number of writers)
    int test_cases[3][2] = {
        {3, 2},
        {5, 5},
        {1, 8}
    };

    // Loop over each test case
    for(int t = 0; t < 3; t++) {
        int num_readers = test_cases[t][0];
        int num_writers = test_cases[t][1];

        // Reset password table and counters before each test
        pthread_mutex_lock(&password_mutex);
        password_count = 0;
        pthread_mutex_unlock(&password_mutex);
        BUFFER = 0;
        read_count = 0;

        pthread_t readers[MAX_THREADS], writers[MAX_THREADS];
        pthread_t dummy_readers[MAX_THREADS], dummy_writers[MAX_THREADS];
        thread_info_t reader_infos[MAX_THREADS], writer_infos[MAX_THREADS];
        thread_info_t dummy_reader_infos[MAX_THREADS], dummy_writer_infos[MAX_THREADS];

        print_table_header(t+1);

        // Create real reader threads
        for(int i = 0; i < num_readers; i++) {
            reader_infos[i].id = i + 1;
            reader_infos[i].is_dummy = 0;
            reader_infos[i].role = 'R';
            if(pthread_create(&readers[i], NULL, reader, &reader_infos[i]) != 0) {
                perror("Failed to create reader thread");
                exit(EXIT_FAILURE);
            }
        }

        // Create real writer threads
        for(int i = 0; i < num_writers; i++) {
            writer_infos[i].id = i + 1;
            writer_infos[i].is_dummy = 0;
            writer_infos[i].role = 'W';
            if(pthread_create(&writers[i], NULL, writer, &writer_infos[i]) != 0) {
                perror("Failed to create writer thread");
                exit(EXIT_FAILURE);
            }
        }

        // Create dummy reader threads equal to real readers
        for(int i = 0; i < num_readers; i++) {
            dummy_reader_infos[i].id = i + 1;
            dummy_reader_infos[i].is_dummy = 1;
            dummy_reader_infos[i].role = 'R';
            if(pthread_create(&dummy_readers[i], NULL, reader, &dummy_reader_infos[i]) != 0) {
                perror("Failed to create dummy reader thread");
                exit(EXIT_FAILURE);
            }
        }

        // Create dummy writer threads equal to real writers
        for(int i = 0; i < num_writers; i++) {
            dummy_writer_infos[i].id = i + 1;
            dummy_writer_infos[i].is_dummy = 1;
            dummy_writer_infos[i].role = 'W';
            if(pthread_create(&dummy_writers[i], NULL, writer, &dummy_writer_infos[i]) != 0) {
                perror("Failed to create dummy writer thread");
                exit(EXIT_FAILURE);
            }
        }

        // Wait for all threads to finish
        for(int i = 0; i < num_readers; i++) {
            pthread_join(readers[i], NULL);
        }
        for(int i = 0; i < num_readers; i++) {
            pthread_join(dummy_readers[i], NULL);
        }
        for(int i = 0; i < num_writers; i++) {
            pthread_join(writers[i], NULL);
        }
        for(int i = 0; i < num_writers; i++) {
            pthread_join(dummy_writers[i], NULL);
        }
    }

    // Cleanup synchronization primitives
    pthread_mutex_destroy(&password_mutex);
    sem_destroy(&resource_access);
    sem_destroy(&read_count_access);

    return 0;
}
