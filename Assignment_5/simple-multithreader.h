
/*#include <iostream>

#include <list>
#include <functional>
#include <stdlib.h>
#include <cstring>
#include <chrono>

// Struct for thread arguments in a 1D loop
typedef struct {
    int low;
    int high;
    std::function<void(int)> lambda;
} thread_args;

// Struct for thread arguments in a 2D loop
typedef struct {
    int low1;
    int high1;
    int low2;
    int high2;
    std::function<void(int, int)> lambda;
} thread_args_matrix;

// Thread function for a 1D loop
void* thread_func(void* ptr) {
    thread_args* args = static_cast<thread_args*>(ptr);
    for (int i = args->low; i < args->high; ++i) {
        args->lambda(i);
    }
    return NULL;
}

// Thread function for a 2D loop
void* thread_func_matrix(void* ptr) {
    thread_args_matrix* args = static_cast<thread_args_matrix*>(ptr);
    for (int i = args->low1; i < args->high1; ++i) {
        for (int j = args->low2; j < args->high2; ++j) {
            args->lambda(i, j);
        }
    }
    return NULL;
}

// Parallel for loop for a 1D range
void parallel_for(int low, int high, std::function<void(int)> lambda, int numThreads) {
    numThreads--;

    // Record start time
    auto start = std::chrono::high_resolution_clock::now();

    // Thread management
    pthread_t tid[numThreads];
    thread_args args[numThreads];
    int chunk = (high - low) / numThreads;

    int counter = numThreads;

    if ((high - low) % numThreads != 0) {
        counter--;
    }

    for (int i = 0; i < counter; i++) {
        args[i].low = i * chunk;
        args[i].high = (i + 1) * chunk;
        args[i].lambda = lambda;
        pthread_create(&tid[i], NULL, thread_func, static_cast<void*>(&args[i]));
    }

    pthread_t residue;
    thread_args data;

    if (counter != numThreads) {
        data.high = high;
        data.low = counter * chunk;
        data.lambda = lambda;
        pthread_create(&residue, NULL, thread_func, static_cast<void*>(&data));
    }

    // Join threads
    for (int i = 0; i < counter; i++) {
        pthread_join(tid[i], NULL);
    }

    if (counter != numThreads) {
        pthread_join(residue, NULL);
    }

    // Record end time
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    // Print execution time
    std::cout << "Execution time: " << duration.count() << " seconds" << std::endl;
}

// Parallel for loop for a 2D range
void parallel_for(int low1, int high1, int low2, int high2,
    std::function<void(int, int)> lambda, int numThreads) {
    numThreads--;

    // Record start time
    auto start = std::chrono::high_resolution_clock::now();

    // Thread management
    pthread_t tid[numThreads];
    thread_args_matrix args[numThreads];
    int chunk1 = (high1 - low1) / numThreads;
    int chunk2 = (high2 - low2) / numThreads;

    int counter = numThreads;

    if ((high1 - low1) % numThreads != 0) {
        counter--;
    }

    for (int i = 0; i < counter; i++) {
        args[i].low1 = i * chunk1;
        args[i].high1 = (i + 1) * chunk1;
        args[i].low2 = low2;
        args[i].high2 = high2;
        args[i].lambda = lambda;
        pthread_create(&tid[i], NULL, thread_func_matrix, static_cast<void*>(&args[i]));
    }

    pthread_t residue;
    thread_args_matrix data;

    if (counter != numThreads) {
        data.high1 = high1;
        data.low1 = counter * chunk1;
        data.low2 = low2;
        data.high2 = high2;
        data.lambda = lambda;

        pthread_create(&residue, NULL, thread_func_matrix, static_cast<void*>(&data));
    }

    // Join threads
    for (int i = 0; i < counter; ++i) {
        pthread_join(tid[i], NULL);
    }

    if (counter != numThreads) {
        pthread_join(residue, NULL);
    }

    // Record end time
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    // Print execution time
    std::cout << "Execution time: " << duration.count() << " seconds" << std::endl;
}

// Main function provided by the user
int user_main(int argc, char **argv);

// Entry point
int main(int argc, char **argv) {
    // Call user's main function
    int rc = user_main(argc, argv);

    // Return the user's main function result
    return rc;
}

// Redefine 'main' to be 'user_main'
#define main user_main*/

#include <iostream>
#include <list>
#include <functional>
#include <stdlib.h>
#include <cstring>
#include <pthread.h>
#include <chrono>

int user_main(int argc, char **argv);

/* Demonstration on how to pass lambda as parameter.
 * "&&" means r-value reference. You may read about it online.
 */
void demonstration(std::function<void()> && lambda) {
  lambda();
}

// Struct for thread arguments in a 1D loop
typedef struct {
    int low;
    int high;
    std::function<void(int)> lambda;
} thread_args;

// Struct for thread arguments in a 2D loop
typedef struct {
    int low1;
    int high1;
    int low2;
    int high2;
    std::function<void(int, int)> lambda;
} thread_args_matrix;

// Thread function for a 1D loop
void* thread_func(void* ptr) {
    thread_args* args = static_cast<thread_args*>(ptr);
    for (int i = args->low; i < args->high; ++i) {
        args->lambda(i);
    }
    return NULL;
}

// Thread function for a 2D loop
void* thread_func_matrix(void* ptr) {
    thread_args_matrix* args = static_cast<thread_args_matrix*>(ptr);
    for (int i = args->low1; i < args->high1; ++i) {
        for (int j = args->low2; j < args->high2; ++j) {
            args->lambda(i, j);
        }
    }
    return NULL;
}

// Parallel for loop for a 1D range
void parallel_for(int low, int high, std::function<void(int)> lambda, int numThreads) {
    if (high <= low) {
        std::cerr << "Error: high must be greater than low" << std::endl;
        return;
    }
    if (numThreads <= 0) {
        std::cerr << "Error: numThreads must be greater than 0" << std::endl;
        return;
    }

    numThreads--;

    // Record start time
    auto start = std::chrono::high_resolution_clock::now();

    // Thread management
    pthread_t tid[numThreads];
    thread_args args[numThreads];
    int chunk = (high - low) / numThreads;

    int counter = numThreads;

    if ((high - low) % numThreads != 0) {
        counter--;
    }

    for (int i = 0; i < counter; i++) {
        args[i].low = low + i * chunk;
        args[i].high = low + (i + 1) * chunk;
        args[i].lambda = lambda;
        int rc = pthread_create(&tid[i], NULL, thread_func, static_cast<void*>(&args[i]));
        if (rc) {
            std::cerr << "Error: unable to create thread, " << rc << std::endl;
            exit(-1);
        }
    }

    pthread_t residue;
    thread_args data;

    if (counter != numThreads) {
        data.high = high;
        data.low = low + counter * chunk;
        data.lambda = lambda;
        int rc = pthread_create(&residue, NULL, thread_func, static_cast<void*>(&data));
        if (rc) {
            std::cerr << "Error: unable to create thread, " << rc << std::endl;
            exit(-1);
        }
    }

    // Join threads
    for (int i = 0; i < counter; i++) {
        int rc = pthread_join(tid[i], NULL);
        if (rc) {
            std::cerr << "Error: unable to join thread, " << rc << std::endl;
            exit(-1);
        }
    }

    if (counter != numThreads) {
        int rc = pthread_join(residue, NULL);
        if (rc) {
            std::cerr << "Error: unable to join thread, " << rc << std::endl;
            exit(-1);
        }
    }

    // Record end time
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    // Print execution time
    std::cout << "Execution time: " << duration.count() << " seconds" << std::endl;
}

// Parallel for loop for a 2D range
void parallel_for(int low1, int high1, int low2, int high2,
    std::function<void(int, int)> lambda, int numThreads) {
    if (high1 <= low1 || high2 <= low2) {
        std::cerr << "Error: high1 and high2 must be greater than low1 and low2 respectively" << std::endl;
        return;
    }
    if (numThreads <= 0) {
        std::cerr << "Error: numThreads must be greater than 0" << std::endl;
        return;
    }

    numThreads--;

    // Record start time
    auto start = std::chrono::high_resolution_clock::now();

    // Thread management
    pthread_t tid[numThreads];
    thread_args_matrix args[numThreads];
    int chunk1 = (high1 - low1) / numThreads;

    int counter = numThreads;

    if ((high1 - low1) % numThreads != 0) {
        counter--;
    }

    for (int i = 0; i < counter; i++) {
        args[i].low1 = low1 + i * chunk1;
        args[i].high1 = low1 + (i + 1) * chunk1;
        args[i].low2 = low2;
        args[i].high2 = high2;
        args[i].lambda = lambda;
        int rc = pthread_create(&tid[i], NULL, thread_func_matrix, static_cast<void*>(&args[i]));
        if (rc) {
            std::cerr << "Error: unable to create thread, " << rc << std::endl;
            exit(-1);
        }
    }

    pthread_t residue;
    thread_args_matrix data;

    if (counter != numThreads) {
        data.high1 = high1;
        data.low1 = low1 + counter * chunk1;
        data.low2 = low2;
        data.high2 = high2;
        data.lambda = lambda;

        int rc = pthread_create(&residue, NULL, thread_func_matrix, static_cast<void*>(&data));
        if (rc) {
            std::cerr << "Error: unable to create thread, " << rc << std::endl;
            exit(-1);
        }
    }

    // Join threads
    for (int i = 0; i < counter; ++i) {
        int rc = pthread_join(tid[i], NULL);
        if (rc) {
            std::cerr << "Error: unable to join thread, " << rc << std::endl;
            exit(-1);
        }
    }

    if (counter != numThreads) {
        int rc = pthread_join(residue, NULL);
        if (rc) {
            std::cerr << "Error: unable to join thread, " << rc << std::endl;
            exit(-1);
        }
    }

    // Record end time
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    // Print execution time
    std::cout << "Execution time: " << duration.count() << " seconds" << std::endl;
}

// Entry point
int main(int argc, char **argv) {
    return user_main(argc, argv);
}

// Redefine 'main' to be 'user_main'
#define main user_main
