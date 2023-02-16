#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_ARGS 10
#define MAX_COMMAND_LENGTH 100

// Global variables
int num_processes = 0;
pid_t child_pids[MAX_ARGS];


void start_process(char **args) {
    pid_t pid = fork();

    if (pid == 0) {
        // Child process
        execvp(args[0], args);
        printf("Command not found: %s\n", args[0]);
        exit(1);
    } else if (pid > 0) {
        // Parent process
        child_pids[num_processes++] = pid;
    } else {
        // Error forking
        printf("Error forking process\n");
    }
}

// Wait for all child processes to exit
void wait_for_all_processes() {
    int i;
    for (i = 0; i < num_processes; i++) {
        waitpid(child_pids[i], NULL, 0);
    }
    num_processes = 0;
}

// Wait for the specified child process to exit
void wait_for_process(pid_t pid) {
    int i;
    for (i = 0; i < num_processes; i++) {
        if (child_pids[i] == pid) {
            waitpid(child_pids[i], NULL, 0);
            // Remove the process from the list
            child_pids[i] = child_pids[num_processes-1];
            num_processes--;
            return;
        }
    }
    printf("Process %d not found\n", pid);
}

// Kill the specified child process
void kill_process(pid_t pid) {
    int i;
    for (i = 0; i < num_processes; i++) {
        if (child_pids[i] == pid) {
            kill(child_pids[i], SIGTERM);
            // Remove the process from the list
            child_pids[i] = child_pids[num_processes-1];
            num_processes--;
            return;
        }
    }
    printf("Process %d not found\n", pid);
}

// Handle the Control-C signal by sending a SIGTERM to the last running process
void handle_control_c(int sig) {
    if (num_processes > 0) {
        kill(child_pids[num_processes-1], SIGTERM);
        num_processes--;
    }
}

// Terminate all child processes, wait for completion, and exit the shell
void quit() {
    int i;
    for (i = 0; i < num_processes; i++) {
        kill(child_pids[i], SIGTERM);
    }
    wait_for_all_processes();
    exit(0);
}

int main() {
    char input[MAX_COMMAND_LENGTH];
    char *args[MAX_ARGS];
    int status;
    
    signal(SIGINT, handle_control_c);

    while (1) {
        printf("> ");
        fgets(input, MAX_COMMAND_LENGTH, stdin);
        // Remove newline character from input
        input[strcspn(input, "\n")] = 0;

        // Parse input into arguments
        int num_args = 0;
        char *arg = strtok(input, " ");
        while (arg != NULL) {
            args[num_args++] = arg;
            arg = strtok(NULL, " ");
        }
        args[num_args] = NULL;

        if (num_args == 0) {
            // No input provided
            continue;
        } else if (strcmp(args[0], "exit") == 0) {
            // Exit the shell
            quit();
        } else if (strcmp(args[0], "start") == 0) {
            // Start a new process with the given arguments
            start_process(&args[1]);
        } else if (strcmp(args[0], "wait") == 0) {
            // Wait for any child process to exit
            waitpid(-1, &status, 0);
        } else if (strcmp(args[0], "waitfor") == 0) {
            // Wait for a specific child process to exit
            if (num_args < 2) {
                printf("Usage: waitfor <pid>\n");
            } else {
                pid_t pid = atoi(args[1]);
                wait_for_process(pid);
            }
        } else if (strcmp(args[0], "run") == 0) {
            // Start a new process and wait for it to exit
            start_process(&args[1]);
            wait_for_all_processes();
        } else if (strcmp(args[0], "kill") == 0) {
            // Kill a specific child process
            if (num_args < 2) {
                printf("Usage: kill <pid>\n");
            } else {
                pid_t pid = atoi(args[1]);
                kill_process(pid);
            }
        } else if (strcmp(args[0], "quit") == 0) {
            // Terminate all child processes and exit the shell
            quit();
        } else if (strcmp(args[0], "bound") == 0) {
            // Run a time-bound process and kill it if it exceeds the time limit
            if (num_args < 3) {
                printf("Usage: bound <time limit> <command>\n");
            } else {
                int time_limit = atoi(args[1]);
                pid_t pid = fork();
                if (pid == 0) {
                    // Child process
                    struct timeval start_time, end_time;
                    gettimeofday(&start_time, NULL);
                    execvp(args[2], &args[2]);
                    printf("Command not found: %s\n", args[2]);
                    exit(1);
                } else if (pid > 0) {
                    // Parent process
                    child_pids[num_processes++] = pid;
                    struct timeval start_time, end_time;
                    gettimeofday(&start_time, NULL);
                    int time_elapsed;
                    while (1) {
                        // Check if the time limit has been exceeded
                        gettimeofday(&end_time, NULL);
                        time_elapsed = (end_time.tv_sec - start_time.tv_sec) * 1000 +
                                       (end_time.tv_usec - start_time.tv_usec) / 1000;
                        if (time_elapsed >= time_limit) {
                            // Kill the process
                                                    kill(pid, SIGTERM);
                        printf("Process timed out after %d ms\n", time_elapsed);
                        break;
                    }
                    
                    // Check if the process has exited
                    int child_status;
                    pid_t result = waitpid(pid, &child_status, WNOHANG);
                    if (result == pid) {
                        // Child process has exited
                        printf("Process exited with status %d\n", child_status);
                        break;
                    }

                    // Sleep for a short time before checking again
                    usleep(10000);
                }
            } else {
                // Forking failed
                printf("Fork failed\n");
            }
        }
        } else {
            printf("Unknown command: %s\n", args[0]);
        }
    }
}

