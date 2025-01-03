#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <time.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <signal.h>
#include "common.h"

pid_t scheduler_pid;        // Global variable for the scheduler process ID
shm_t *shared_memory;       // Global pointer to the shared memory structure


//  Data-Structure to store command history details
typedef struct {
    char *command;
    pid_t pid;
    time_t start_time;
    time_t end_time;
    time_t wait_time;
}CommandHistory;
#define MAX_HISTORY 100
// Store command history with details
CommandHistory history[MAX_HISTORY];
int history_count = 0;

// Function to add command to history
void add_history(char *cmd, pid_t pid, time_t start_time) {
    if (history_count < MAX_HISTORY) {
       history[history_count].command = malloc(strlen(cmd) + 1);
        if (history[history_count].command != NULL) {
            for (int i = 0; cmd[i] != '\0'; i++) {
                history[history_count].command[i] = cmd[i];
            }
            history[history_count].command[strlen(cmd)] = '\0'; // null character termination ('\0')
        }
        history[history_count].pid = pid;
        history[history_count].start_time = start_time;
        history[history_count].end_time = 0; // Record end time
        history[history_count].wait_time = 0; // Record end time

        history_count++;
    }
}

// Function for updating command end time in history
void update_history_end_time(pid_t pid) {
    for (int i = 0; i < history_count; i++) {
        if (history[i].pid == pid) {
            history[i].end_time = time(NULL);
            break;
        }
    }
}

// Function to display command execution details
void display_execution_details() {
    printf("\nCommand Execution Details:\n");
    for (int i = 0; i < history_count; i++) {
        if (history[i].end_time > 0) { // Only show commands that have completed
            double duration = difftime(history[i].end_time, history[i].start_time);
            printf("Command: %s\n", history[i].command);
            printf("PID: %d\n", history[i].pid);
            printf("Start Time: %s", ctime(&history[i].start_time));
            printf("End Time: %s", ctime(&history[i].end_time));
            printf("End Time: %s", ctime(&history[i].wait_time));
            
        }
    }
}

void handle_sigchld(int sig) {
    
    pid_t pid;
    int status;

    // Wait for all child processes that have terminated
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // Only update if the process was a background process.
        printf(" process %d terminated\n", pid);
        update_history_end_time(pid); // Update end time for background commands
    }

    
}

void execute_command(char *command) {
    char *args[150];
    int i = 0;

    // Tokenize the command
    char *token = strtok(command, " ");
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    // Stop this process before executing the command
    kill(getpid(), SIGSTOP);  // Stop the child process

    // Execute the command
    if (execvp(args[0], args) < 0) {
        perror("execvp failed");
        exit(EXIT_FAILURE);
    }
}

void handle_single_command(char *command, int background) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        
        execute_command(command);
    } else {
       
        if (!background) {
            
            waitpid(pid, NULL, 0);
        } else {
            printf("Background process PID: %d\n", pid);
        }
    }
}


void handle_sigint(int sig) {
    if (scheduler_pid > 0) {
        kill(scheduler_pid, SIGINT);
    }
    cleanup(shared_memory);  // Ensure shared memory cleanup
    printf("\nReceived SIGINT, cleaning up and exiting...\n");
    exit(0);
}

void parseInput(char *input, char *parameters[], int *counter) {
    *counter = 0;
    char *token = strtok(input, " ");
    while (token != NULL) {
        parameters[(*counter)++] = token;
        token = strtok(NULL, " ");
    }
}




int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <ncpu> <time_slice>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int ncpu = atoi(argv[1]);
    int time_slice = atoi(argv[2]);

    shared_memory = setup();
    if (shared_memory == MAP_FAILED) {
        exit(EXIT_FAILURE);
    }
    strcpy(shared_memory->file_name, "");
    for (int i = 0; i < 128; i++) {
        shared_memory->running[i] = false;
    }
    shared_memory->counter = 0;
    shared_memory->ncpu = ncpu;
    shared_memory->tslice = time_slice;

    signal(SIGINT, handle_sigint);

    scheduler_pid = fork();
    if (scheduler_pid < 0) {
        perror("Fork error");
        cleanup(shared_memory);
        exit(EXIT_FAILURE);
    } else if (scheduler_pid == 0) {
        shm_t *scheduler_memory = setup();
        if (scheduler_memory == MAP_FAILED) {
            exit(EXIT_FAILURE);
        }

        char *args[4] = {"./simple_scheduler", argv[1], argv[2], NULL};
        execvp(args[0], args);
        perror("Error in exec function");
        exit(EXIT_FAILURE);
    }

    char input[MAX_INPUT_SIZE];
    char *parameters[MAX_INPUT_SIZE];
    int counter;

    while (true) {
        printf("SimpleShell$ ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Error reading input.\n");
            continue;
        }

        input[strcspn(input, "\n")] = '\0';
        if (strlen(input) == 0) {
            printf("Empty command. Please enter a valid command.\n");
            continue;
        }

        if (strcmp(input, "1") == 0) {
            handle_single_command(input, 0);
            handle_single_command(input, 1);
            continue;
        }

        char *pipe_check = strchr(input, '|');
        if (pipe_check != NULL) {
            printf("Pipes are not supported. Use a simple command.\n");
            continue;
        }

        char input_copy[MAX_INPUT_SIZE];
        strcpy(input_copy, input);

        char *token = strtok(input_copy, " ");
        if (strcmp(token, "submit") == 0) {
            token = strtok(NULL, " ");
            if (!token) {
                printf("Invalid submit command.\n");
                continue;
            }

            char file_name[MAX_INPUT_SIZE];
            strcpy(file_name, token);

            int priority = 1;
            token = strtok(NULL, " ");
            if (token && atoi(token) >= 1 && atoi(token) <= 4) {
                priority = atoi(token);
            } else if (token) {
                printf("Invalid priority. Using default priority 1.\n");
            }

            strcat(shared_memory->file_name, file_name);
            strcat(shared_memory->file_name, "|");
            shared_memory->running[shared_memory->counter++] = true;
        } else {
            printf("Invalid command.\n");
        }
    }

    cleanup(shared_memory);
    return 0;
}
