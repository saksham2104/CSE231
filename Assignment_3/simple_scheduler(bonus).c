
//history and sigint needs to be reviewed
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <time.h>
#include <signal.h>
#include "common.h"

#define MAX_QUEUE_SIZE 128

typedef struct Process {
    int priority; //1 to 4 priority
    int finished; // 1 indicates finished
    int pid;
    char name[1000];
    struct timespec start_time;
    long long exec_time;
    int no_cycles;
    long long wait_time;
} Process;

typedef struct {
    Process items[MAX_QUEUE_SIZE];
    int front, rear;
} Queue;

Process history[128] = {0};
int history_counter = 0;
int sigint_received = 0; // Flag to check if SIGINT was received

void initializeQueue(Queue *q) {
    q->front = -1;
    q->rear = -1;
    printf("Queue initialized.\n");
}

bool isEmpty(Queue *q) {
    return q->front == -1;
}

bool isFull(Queue *q) {
    return (q->rear + 1) % MAX_QUEUE_SIZE == q->front;
}

void enqueue(Queue *q, Process item) {
    if (isFull(q)) {
        printf("Queue is full. Cannot enqueue any more items.\n");
    } else {
        if (isEmpty(q)) {
            q->front = 0;
        }
        q->rear = (q->rear + 1) % MAX_QUEUE_SIZE;
        q->items[q->rear] = item;
        printf("Enqueued process: %s with PID: %d\n", item.name, item.pid);
    }
}

Process dequeue(Queue *q) {
    Process item;
    if (isEmpty(q)) {
        printf("Queue is empty. Cannot dequeue any items.\n");
        item.pid = -1;
    } else {
        item = q->items[q->front];
        printf("Dequeued process: %s with PID: %d\n", item.name, item.pid);
        if (q->front == q->rear) {
            initializeQueue(q);
        } else {
            q->front = (q->front + 1) % MAX_QUEUE_SIZE;
        }
    }
    return item;
}

long long timespec_diff(struct timespec start, struct timespec end) {
    long long diff = (end.tv_sec - start.tv_sec) * 1000LL;
    diff += (end.tv_nsec - start.tv_nsec) / 1000000LL;
    return diff;
}

void sigintHandler(int sig_num) {
    sigint_received = 1; // Set flag to indicate SIGINT received
    printf("SIGINT received. Will finish remaining processes before exit.\n");
}

typedef struct {
    Process *data;
    size_t size;
    size_t capacity;
} DynamicArray;

void initArray(DynamicArray *arr, size_t initialCapacity) {
    arr->data = malloc(initialCapacity * sizeof(Process));
    arr->size = 0;
    arr->capacity = initialCapacity;
}

void push_back(DynamicArray *arr, Process value) {
    if (arr->size == arr->capacity) {
        arr->capacity *= 2;
        arr->data = realloc(arr->data, arr->capacity * sizeof(Process));
        if (!arr->data) {
            perror("Failed to reallocate memory");
            exit(1);
        }
        //printf("Dynamic array capacity increased to %zu.\n", arr->capacity);
    }
    arr->data[arr->size++] = value;
    printf("Added process %s to dynamic array. Current size: %zu.\n", value.name, arr->size);
}

void freeArray(DynamicArray *arr) {
    free(arr->data);
    arr->data = NULL;
    arr->size = 0;
    arr->capacity = 0;
}

void printProcessHistory() {
    printf("Process History:\n");
    int counter = 1;
    for (int i = 0; i < history_counter; i++) {
        Process *p = &history[i];
        
        printf("%d.\n", counter++);
        printf("File name: %s\n", p->name);
        printf("PID: %d\n", p->pid);

        // Display execution time and waiting time in milliseconds
        printf("Execution time: %lld ms\n", p->exec_time);
        printf("Waiting time: %lld ms\n", p->wait_time);
    }
}

int main(int argc, char *argv[]) {
    struct sigaction sa;
    sa.sa_handler = sigintHandler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error while setting up SIGINT");
        return 1;
    }

    shm_t *shared_memory = setup(); // setup function from common.h
    printf("Shared memory setup completed.\n");

    int num_cpu = shared_memory->ncpu;
    int tslice = shared_memory->tslice;
    printf("Number of CPUs: %d, Time slice: %d ms\n", num_cpu, tslice);

    Queue *q = (Queue *)malloc(sizeof(Queue));
    initializeQueue(q);

    Queue *q_1 = (Queue *)malloc(sizeof(Queue));
    initializeQueue(q_1);

    Queue *q_2 = (Queue *)malloc(sizeof(Queue));
    initializeQueue(q_2);

    Queue *q_3 = (Queue *)malloc(sizeof(Queue));
    initializeQueue(q_3);

    Queue *q_4 = (Queue *)malloc(sizeof(Queue));
    initializeQueue(q_4);





    int is_waiting = 0;

    while (true) {
        if (sigint_received && isEmpty(q) && is_waiting == 0) {
            break; // Exit when all processes are finished and SIGINT was received
        }

        DynamicArray currently_executing;
        initArray(&currently_executing, 1);
        char *tokens[128];
        char input_1[1000];
        strcpy(input_1, shared_memory->priority);
        int *priorities[128]; //add stuff to the priorites array spliting via strtok
        char *token_1=strtok(input_1, "|")
        int count_1=0;
        while(token_1 !=NULL && count_1 < 128){
            priorities[count_1]=atoi(token_1);
            token_1=strtok(NULL,"|");
            count_1++;
        }

        char input[1000];
        strcpy(input, shared_memory->file_name);
        char *token = strtok(input, "|");
        int count = 0;
        while (token != NULL && count < 128) {
            tokens[count] = token;
            token = strtok(NULL, "|");
            count++;
        }

        if (count > 0) {
            for (int i = 0; i < 128; i++) {
                if (shared_memory->running[i]) {
                    Process p = {0};
                    strcpy(p.name, tokens[i]);
                    strcpy(p.priority,priorities[i]);
                    if(p.priority==1){
                        enqueue(q_1,p);
                    }
                    else if(p.priority == 2){
                        enqueue(q_2,p);
                    }
                    else if(p.priority == 3){
                        enqueue(q_3,p);
                    }
                    else if(p.priority == 4){
                        enqueue(q_4,p);
                    }
                    enqueue(q, p);
                    struct timespec start;
                    clock_gettime(CLOCK_MONOTONIC, &start);
                    history[history_counter++] = p;
                    history[history_counter - 1].start_time = start;
                    history[history_counter - 1].no_cycles = 1;
                    shared_memory->running[i] = false;
                    is_waiting++;
                    //printf("Process %s (index %d) is now waiting.\n", p.name, i);
                }
            }
            if (is_waiting > 0) {
                int total = 0;
                while (total < num_cpu && !isEmpty(q)) {
                    //logic needs to be changed like first check if q_4 mei stuff hei then q_3,q_2,q_1 keep track of how many have we taken out via total
                    Process p = dequeue(q);
                    if (p.finished == 1) {
                        printf("Process %s is already finished.\n", p.name);
                        continue;
                    } else {
                        total++;
                        is_waiting--;
                        if (p.pid == 0) {
                            int status = fork();
                            if (status < 0) {
                                printf("Error in forking for process %s\n", p.name);
                                return 1;
                            } else if (status == 0) {
                                char *args[] = {p.name, NULL};
                                push_back(&currently_executing, p);
                                execvp(args[0], args);
                                perror("Error during execvp");
                                exit(EXIT_FAILURE);
                            }
                        } else {
                            for (int j = 0; j < 128; j++) {
                                if (strcmp(p.name, history[j].name) == 0) {
                                    history[j].no_cycles++;
                                }
                            }
                            kill(p.pid, SIGCONT);
                        }
                    }
                }
                usleep(tslice * 1000);

                for (size_t i = 0; i < currently_executing.size; i++) {
                    kill(currently_executing.data[i].pid, SIGSTOP);
                }

                for (size_t i = 0; i < currently_executing.size; i++) {
                    int status;
                    enqueue(q, currently_executing.data[i]);
                    if (waitpid(currently_executing.data[i].pid, &status, WNOHANG) != 0) {
                        currently_executing.data[i].finished = 1;
                        struct timespec end;
                        clock_gettime(CLOCK_MONOTONIC, &end);
                        for (int j = 0; j < 128; j++) {
                            if (strcmp(currently_executing.data[i].name, history[j].name) == 0) {
                                history[j].finished = 1;
                                history[j].exec_time = timespec_diff(history[j].start_time, end);
                                // Calculate waiting time if needed
                                // Assuming waiting time is cumulative for multiple rounds
                                if (history[j].no_cycles > 1) {
                                    history[j].wait_time += timespec_diff(history[j].start_time, end) - history[j].exec_time;
                                }
                            }
                        }
                        printf("Process %s finished execution.\n", currently_executing.data[i].name);
                    }
                }
            }
            freeArray(&currently_executing);
        }
    }

    printProcessHistory();
    cleanup(shared_memory);
    free(q);
    return 0;
}
/*
//history and sigint needs to be reviewed
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <time.h>
#include <signal.h>
#include "common.h"

#define MAX_QUEUE_SIZE 128

typedef struct Process {
    int finished; // 1 indicates finished
    int pid;
    char name[1000];
    struct timespec start_time;
    long long exec_time;
    int no_cycles;
    long long wait_time;
} Process;

typedef struct {
    Process items[MAX_QUEUE_SIZE];
    int front, rear;
} Queue;

Process history[128] = {0};
int history_counter = 0;
int sigint_received = 0; // Flag to check if SIGINT was received

void initializeQueue(Queue *q) {
    q->front = -1;
    q->rear = -1;
    printf("Queue initialized.\n");
}

bool isEmpty(Queue *q) {
    return q->front == -1;
}

bool isFull(Queue *q) {
    return (q->rear + 1) % MAX_QUEUE_SIZE == q->front;
}

void enqueue(Queue *q, Process item) {
    if (isFull(q)) {
        printf("Queue is full. Cannot enqueue any more items.\n");
    } else {
        if (isEmpty(q)) {
            q->front = 0;
        }
        q->rear = (q->rear + 1) % MAX_QUEUE_SIZE;
        q->items[q->rear] = item;
        printf("Enqueued process: %s with PID: %d\n", item.name, item.pid);
    }
}

Process dequeue(Queue *q) {
    Process item;
    if (isEmpty(q)) {
        printf("Queue is empty. Cannot dequeue any items.\n");
        item.pid = -1;
    } else {
        item = q->items[q->front];
        printf("Dequeued process: %s with PID: %d\n", item.name, item.pid);
        if (q->front == q->rear) {
            initializeQueue(q);
        } else {
            q->front = (q->front + 1) % MAX_QUEUE_SIZE;
        }
    }
    return item;
}

long long timespec_diff(struct timespec start, struct timespec end) {
    long long diff = (end.tv_sec - start.tv_sec) * 1000LL;
    diff += (end.tv_nsec - start.tv_nsec) / 1000000LL;
    return diff;
}

void sigintHandler(int sig_num) {
    sigint_received = 1; // Set flag to indicate SIGINT received
    printf("SIGINT received. Will finish remaining processes before exit.\n");
}

typedef struct {
    Process *data;
    size_t size;
    size_t capacity;
} DynamicArray;

void initArray(DynamicArray *arr, size_t initialCapacity) {
    arr->data = malloc(initialCapacity * sizeof(Process));
    arr->size = 0;
    arr->capacity = initialCapacity;
}

void push_back(DynamicArray *arr, Process value) {
    if (arr->size == arr->capacity) {
        arr->capacity *= 2;
        arr->data = realloc(arr->data, arr->capacity * sizeof(Process));
        if (!arr->data) {
            perror("Failed to reallocate memory");
            exit(1);
        }
        printf("Dynamic array capacity increased to %zu.\n", arr->capacity);
    }
    arr->data[arr->size++] = value;
    printf("Added process %s to dynamic array. Current size: %zu.\n", value.name, arr->size);
}

void freeArray(DynamicArray *arr) {
    free(arr->data);
    arr->data = NULL;
    arr->size = 0;
    arr->capacity = 0;
}

void printProcessHistory() {
    printf("Process History:\n");
    int counter = 1;
    for (int i = 0; i < history_counter; i++) {
        Process *p = &history[i];
        
        printf("%d.\n", counter++);
        printf("File name: %s\n", p->name);
        printf("PID: %d\n", p->pid);

        // Display execution time and waiting time in milliseconds
        printf("Execution time: %lld ms\n", p->exec_time);
        printf("Waiting time: %lld ms\n", p->wait_time);
    }
}

int main(int argc, char *argv[]) {
    struct sigaction sa;
    sa.sa_handler = sigintHandler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error while setting up SIGINT");
        return 1;
    }

    shm_t *shared_memory = setup(); // setup function from common.h
    printf("Shared memory setup completed.\n");

    int num_cpu = shared_memory->ncpu;
    int tslice = shared_memory->tslice;
    printf("Number of CPUs: %d, Time slice: %d ms\n", num_cpu, tslice);

    Queue *q = (Queue *)malloc(sizeof(Queue));
    initializeQueue(q);

    int is_waiting = 0;

    while (true) {
        if (sigint_received && isEmpty(q) && is_waiting == 0) {
            break; // Exit when all processes are finished and SIGINT was received
        }

        DynamicArray currently_executing;
        initArray(&currently_executing, 1);
        char *tokens[128];
        char input[1000];
        strcpy(input, shared_memory->file_name);
        char *token = strtok(input, "|");
        int count = 0;
        while (token != NULL && count < 128) {
            tokens[count] = token;
            token = strtok(NULL, "|");
            count++;
        }

        if (count > 0) {
            for (int i = 0; i < 128; i++) {
                if (shared_memory->running[i]) {
                    Process p = {0};
                    strcpy(p.name, tokens[i]);
                    enqueue(q, p);
                    struct timespec start;
                    clock_gettime(CLOCK_MONOTONIC, &start);
                    history[history_counter++] = p;
                    history[history_counter - 1].start_time = start;
                    history[history_counter - 1].no_cycles = 1;
                    shared_memory->running[i] = false;
                    is_waiting++;
                    printf("Process %s (index %d) is now waiting.\n", p.name, i);
                }
            }
            if (is_waiting > 0) {
                int total = 0;
                while (total < num_cpu && !isEmpty(q)) {
                    Process p = dequeue(q);
                    if (p.finished == 1) {
                        printf("Process %s is already finished.\n", p.name);
                        continue;
                    } else {
                        total++;
                        is_waiting--;
                        if (p.pid == 0) {
                            int status = fork();
                            if (status < 0) {
                                printf("Error in forking for process %s\n", p.name);
                                return 1;
                            } else if (status == 0) {
                                char *args[] = {p.name, NULL};
                                push_back(&currently_executing, p);
                                execvp(args[0], args);
                                perror("Error during execvp");
                                exit(EXIT_FAILURE);
                            }
                        } else {
                            for (int j = 0; j < 128; j++) {
                                if (strcmp(p.name, history[j].name) == 0) {
                                    history[j].no_cycles++;
                                }
                            }
                            kill(p.pid, SIGCONT);
                        }
                    }
                }
                usleep(tslice * 1000);

                for (size_t i = 0; i < currently_executing.size; i++) {
                    kill(currently_executing.data[i].pid, SIGSTOP);
                }

                for (size_t i = 0; i < currently_executing.size; i++) {
                    int status;
                    enqueue(q, currently_executing.data[i]);
                    if (waitpid(currently_executing.data[i].pid, &status, WNOHANG) != 0) {
                        currently_executing.data[i].finished = 1;
                        struct timespec end;
                        clock_gettime(CLOCK_MONOTONIC, &end);
                        for (int j = 0; j < 128; j++) {
                            if (strcmp(currently_executing.data[i].name, history[j].name) == 0) {
                                history[j].finished = 1;
                                history[j].exec_time = timespec_diff(history[j].start_time, end);
                                
                                // Calculate waiting time if needed
                                // Assuming waiting time is cumulative for multiple rounds
                                if (history[j].no_cycles > 1) {
                                    history[j].wait_time += timespec_diff(history[j].start_time, end) - history[j].exec_time;
                                }
                            }
                        }
                        printf("Process %s finished execution.\n", currently_executing.data[i].name);
                    }
                }
            }
            freeArray(&currently_executing);
        }
    }

    printProcessHistory();
    cleanup(shared_memory);
    free(q);
    return 0;
}
