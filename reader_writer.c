#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <dispatch/dispatch.h>
#include <unistd.h>
#include <stdint.h>

#define MAX_THREADS 9
#define OPERATIONS 5

// Global shared resource
int BUFFER = 0;

// Password table and mutex for exclusive access
unsigned long password_table[2 * MAX_THREADS];  // max real threads + dummy threads
int password_count = 0;
pthread_mutex_t password_mutex;

// Synchronization semaphores for BUFFER access
dispatch_semaphore_t resource_access;     // controls access to BUFFER
dispatch_semaphore_t read_count_access;   // protects read_count variable
int read_count = 0;

// Struct to hold thread info
typedef struct {
    int id;          // Thread number (1..n)
    int is_dummy;    // 0 = real thread, 1 = dummy thread
    char role;       // 'R' for reader, 'W' for writer
} thread_info_t;

// Hash function for pthread_t to unsigned long
unsigned long hash_pthread_t(pthread_t thread_id) {
    return (unsigned long)(uintptr_t)thread_id;
}

// Add password to the table (thread-safe)
void add_password(unsigned long hash) {
    pthread_mutex_lock(&password_mutex);
    // Check duplicate (for safety)
    for(int i = 0; i < password_count; i++) {
        if(password_table[i] == hash) {
            pthread_mutex_unlock(&password_mutex);
            return; // Already added
        }
    }
    if(password_count < 2 * MAX_THREADS) {
        password_table[password_count++] = hash;
    } else {
        fprintf(stderr, "Password table full!\n");
        // exit or handle error as needed
    }
    pthread_mutex_unlock(&password_mutex);
}

// Check if thread hash is authorized
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

// Print header for table output per test case
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

    if(!info->is_dummy) {
        add_password(hash);
    }

    for(int i = 0; i < OPERATIONS; i++) {
        // Authorization check for real threads only
        if(!info->is_dummy && !is_authorized(hash)) {
            fprintf(stderr, "Reader Thread %d: Unauthorized access!\n", info->id);
            pthread_exit(NULL);
        }

        // Reader entry section
        dispatch_semaphore_wait(read_count_access, DISPATCH_TIME_FOREVER);
        read_count++;
        if(read_count == 1) {
            dispatch_semaphore_wait(resource_access, DISPATCH_TIME_FOREVER);
        }
        dispatch_semaphore_signal(read_count_access);

        // Critical section: read BUFFER
        printf("%-10d %-15lu %-10s %-10s %-15d\n",
               info->id,
               hash,
               info->is_dummy ? "dummy" : "real",
               "reader",
               BUFFER);

        sleep(1); // simulate reading time

        // Reader exit section
        dispatch_semaphore_wait(read_count_access, DISPATCH_TIME_FOREVER);
        read_count--;
        if(read_count == 0) {
            dispatch_semaphore_signal(resource_access);
        }
        dispatch_semaphore_signal(read_count_access);

        sleep(1); // wait before next operation
    }

    pthread_exit(NULL);
}

// Writer thread function
void* writer(void* arg) {
    thread_info_t *info = (thread_info_t*) arg;
    unsigned long hash = hash_pthread_t(pthread_self());

    if(!info->is_dummy) {
        add_password(hash);
    }

    for(int i = 0; i < OPERATIONS; i++) {
        // Authorization check for real threads only
        if(!info->is_dummy && !is_authorized(hash)) {
            fprintf(stderr, "Writer Thread %d: Unauthorized access!\n", info->id);
            pthread_exit(NULL);
        }

        // Writer entry section
        dispatch_semaphore_wait(resource_access, DISPATCH_TIME_FOREVER);

        // Critical section: write random value to BUFFER
        BUFFER = rand() % 10000;

        printf("%-10d %-15lu %-10s %-10s %-15d\n",
               info->id,
               hash,
               info->is_dummy ? "dummy" : "real",
               "writer",
               BUFFER);

        sleep(1); // simulate writing time

        // Writer exit section
        dispatch_semaphore_signal(resource_access);

        sleep(1); // wait before next operation
    }

    pthread_exit(NULL);
}

int main() {
    srand((unsigned int)time(NULL));
    pthread_mutex_init(&password_mutex, NULL);
    resource_access = dispatch_semaphore_create(1);
    read_count_access = dispatch_semaphore_create(1);

    // Test cases: array of (readers, writers)
    int test_cases[3][2] = {
        {3, 2},  // Test 1: 3 readers, 2 writers
        {5, 5},  // Test 2: 5 readers, 5 writers
        {1, 8}   // Test 3: 1 reader, 8 writers
    };

    for(int t = 0; t < 3; t++) {
        int num_readers = test_cases[t][0];
        int num_writers = test_cases[t][1];

        // Reset password table and counters
        pthread_mutex_lock(&password_mutex);
        password_count = 0;
        pthread_mutex_unlock(&password_mutex);
        BUFFER = 0;
        read_count = 0;

        // Arrays for threads and their info
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

        // Create dummy reader threads (equal number to real readers)
        for(int i = 0; i < num_readers; i++) {
            dummy_reader_infos[i].id = i + 1;
            dummy_reader_infos[i].is_dummy = 1;
            dummy_reader_infos[i].role = 'R';
            if(pthread_create(&dummy_readers[i], NULL, reader, &dummy_reader_infos[i]) != 0) {
                perror("Failed to create dummy reader thread");
                exit(EXIT_FAILURE);
            }
        }

        // Create dummy writer threads (equal number to real writers)
        for(int i = 0; i < num_writers; i++) {
            dummy_writer_infos[i].id = i + 1;
            dummy_writer_infos[i].is_dummy = 1;
            dummy_writer_infos[i].role = 'W';
            if(pthread_create(&dummy_writers[i], NULL, writer, &dummy_writer_infos[i]) != 0) {
                perror("Failed to create dummy writer thread");
                exit(EXIT_FAILURE);
            }
        }

        // Join real reader threads
        for(int i = 0; i < num_readers; i++) {
            pthread_join(readers[i], NULL);
        }
        // Join dummy reader threads
        for(int i = 0; i < num_readers; i++) {
            pthread_join(dummy_readers[i], NULL);
        }
        // Join real writer threads
        for(int i = 0; i < num_writers; i++) {
            pthread_join(writers[i], NULL);
        }
        // Join dummy writer threads
        for(int i = 0; i < num_writers; i++) {
            pthread_join(dummy_writers[i], NULL);
        }
    }

    pthread_mutex_destroy(&password_mutex);

    return 0;
}
