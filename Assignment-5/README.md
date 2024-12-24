Contibutions:Saksham:Thread Functions,debugging and documentation
             Atin: Error Handling , parallel_for function ,debugging 

Flow of the code:
    This code provides a framework for executing parallel loops in both 1D and 2D ranges using multi-threading with POSIX threads (pthread)

1. Definitions

#define main user_main:

Redefines main to user_main. This allows customization of the entry point logic while maintaining flexibility in testing or embedding.

2. Lambda Demonstration
demonstration():
Accepts a lambda function passed as an r-value reference (&&) and invokes it.
Illustrates how to pass and execute lambdas, but it is not directly used in parallel execution logic.

3. Thread Argument Structures
1D Loop (thread_args):
Holds the range (low, high) and the lambda function to execute for the loop.
2D Loop (thread_args_matrix):
Holds two ranges (low1, high1 for the outer loop, low2, high2 for the inner loop) and the lambda function.

4. Thread Functions
1D Loop (thread_func):
Processes the specified range of indices in a loop and invokes the lambda for each index.
2D Loop (thread_func_matrix):
Processes two nested loops (outer and inner ranges) and invokes the lambda for each pair of indices.


This code provides a framework for executing parallel loops in both 1D and 2D ranges using multi-threading with POSIX threads (pthread). Here's the flow of the code:

1. Includes and Definitions
Headers:

iostream: For input/output operations.
list: Standard Template Library (STL) list for potential data structure usage.
functional: To use std::function for lambda expressions.
stdlib.h and cstring: Standard C libraries for general utilities and string operations.
pthread.h: For multi-threading functionalities using POSIX threads.
chrono: For measuring execution time.
#define main user_main:

Redefines main to user_main. This allows customization of the entry point logic while maintaining flexibility in testing or embedding.
2. Lambda Demonstration
demonstration():
Accepts a lambda function passed as an r-value reference (&&) and invokes it.
Illustrates how to pass and execute lambdas, but it is not directly used in parallel execution logic.
3. Thread Argument Structures
1D Loop (thread_args):
Holds the range (low, high) and the lambda function to execute for the loop.
2D Loop (thread_args_matrix):
Holds two ranges (low1, high1 for the outer loop, low2, high2 for the inner loop) and the lambda function.
4. Thread Functions
1D Loop (thread_func):
Processes the specified range of indices in a loop and invokes the lambda for each index.
2D Loop (thread_func_matrix):
Processes two nested loops (outer and inner ranges) and invokes the lambda for each pair of indices.
5. Parallel Execution Functions
1D Parallel Loop (parallel_for(low, high, lambda, numThreads)):

Divides the range (low, high) into chunks and assigns each chunk to a separate thread.
Handles residual indices if the range is not evenly divisible by numThreads.
Creates threads using pthread_create and waits for their completion with pthread_join.
Measures execution time and prints it.
2D Parallel Loop (parallel_for(low1, high1, low2, high2, lambda, numThreads)):

Similar logic but for nested loops with ranges (low1, high1) and (low2, high2).

6. Execution Time Measurement
Both parallel_for functions measure execution time using std::chrono, providing feedback on parallelization efficiency.
7. Main Entry Point
main():
Redirects to user_main, which will implement the program logic.
user_main acts as a placeholder for the user-defined entry point, facilitating flexibility and customization.
    General Workflow
    Setup:

    The user invokes either the 1D or 2D parallel_for function, passing appropriate parameters (ranges, lambda, thread count).
    Thread Management:

    The range(s) are divided among threads. Threads execute their respective portions of the task.
    Execution:

    Each thread runs independently, processing its range and invoking the provided lambda function.
    Synchronization:

    The main thread waits for all threads to complete using pthread_join.
    Feedback:

    Execution time is calculated and displayed for performance evaluation.
Key Features
Threading Efficiency:
Implements dynamic handling of residual indices for optimal load distribution.
Flexibility:
Supports custom user-defined lambda functions for both 1D and 2D iterations.
Performance Insight:
Reports execution time for benchmarking purposes.
Code Reusability:
Modular functions and customizable entry point (user_main) make it reusable across projects.
This flow ensures a scalable, parallelized solution for executing repetitive tasks efficiently.
GITHUB LINK:https://github.com/saksham2104/OS_Assignment-5
