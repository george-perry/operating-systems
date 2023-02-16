#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/types.h>

// Global variables
int num_processes = 0;
pid_t child_pids[10];
pid_t last_pid;

// Command to start a certain process
void start_process(char **args) {
    pid_t pid;
    pid = fork();
    if (pid == 0) {
        execvp(args[0], args);
        printf("ndshell: Error - command not found: %s\n", args[0]);
        exit(1);
    } 
    
    else if (pid < 0) {
        // Forking failed
        printf("ndshell: error fork failed\n");
    } 
    
    else {
        // Parent process
        child_pids[num_processes++] = pid;
        printf("ndshell: process %d started\n", pid);
        last_pid = pid;
    }
}

// Wait for all child processes
void wait_for_all_processes() {
    int status;
    pid_t child_pid;
    child_pid = waitpid(-1, &status, 0);

    if (child_pid == -1) {
        printf("ndshell: No children.\n");
    } else {
        // Check if process exitied normally using WIFEXITED / WIFSIGNALED, print status, 
        if (WIFEXITED(status)) {
            printf("ndshell: process %d exited normally with status %d\n", child_pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("ndshell: process %d exited abnormally with signal %d: %s\n", child_pid, WTERMSIG(status), strsignal(WTERMSIG(status)));
        }
    }
}

// Wait for the specified child process to exit and print messages accordingly
void wait_for_process(pid_t pid) {
    int status;
    pid_t result = waitpid(pid, &status, 0);
    if (result == -1) {
        printf("ndshell: No such process.\n");
    } 
    
    else if (WIFEXITED(status)) {
        printf("ndshell: process %d exited normally with status %d\n", pid, WEXITSTATUS(status));
    } 
    
    else if (WIFSIGNALED(status)) {
        printf("ndshell: process %d exited abnormally with signal %d: %s.\n", pid, WTERMSIG(status), strsignal(WTERMSIG(status)));
    } 
    
    else {
        printf("ndshell: unknown exit status\n");
    }
}

// Kill the specified child process and print messages accordingly
void kill_process(pid_t pid) {

    int status;
    // Check if error occurs when sending kill signal
    if (kill(pid, SIGKILL) == -1) {
        printf("ndshell: Error - failed to kill process with PID %d\n", pid);
        exit(1);
    }

    if (waitpid(pid, &status, 0) == -1) {
        printf("ndshell: Error - failed to wait for process with PID %d to exit\n", pid);
        exit(1);
    }

    if (WIFEXITED(status)) {
        printf("ndshell: process %d exited normally with status %d\n", pid, WEXITSTATUS(status));
    } 
    
    else if (WIFSIGNALED(status)) {
        printf("ndshell: process %d exited abnormally with signal %d: %s\n", pid, WTERMSIG(status), strsignal(WTERMSIG(status)));
    }
}


// Handle the Control-C signal by sending a SIGTERM to the last running process
void handle_control_c(int sig) {
    if (last_pid > 0) {
        printf("Sending SIGTERM to process %d\n", last_pid);
        kill(last_pid, SIGTERM);
        last_pid = 0;
    }

    printf("\nndshell> ");
    fflush(stdout);
}

// Terminate all child processes, wait for completion, and exit the shell
void quit() {
    int i;
    for (i = 0; i < num_processes; i++) {
        kill(child_pids[i], SIGTERM);
        wait_for_process(child_pids[i]);
    }
    printf("All child processes complete – exiting the shell.\n");
    exit(0);
}

// Takes a timeout and checks if command takes too long
void bound(int timeout, char **args) {
    pid_t pid = fork();
    if (pid < 0) {
        printf("ndshell: error fork failed\n");
        return;
    } 
    
    else if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            printf("ndshell: Error - command not found: %s\n", args[0]);
            exit(EXIT_FAILURE);
        }

    } 
    
    else {
        // Parent process
        int status;
        int result = waitpid(pid, &status, WNOHANG);
        int time_elapsed = 0;
        while (result == 0 && time_elapsed < timeout) {
            // Wait for a short period of time
            sleep(1);
            time_elapsed++;

            // Check if child process has exited
            result = waitpid(pid, &status, WNOHANG);
        }

        if (result == 0) {
            // Since the child process is still running kill it
            printf("ndshell: process %d exceeded the time limit, killing it...\n", pid);
            kill_process(pid);

            // Wait to exit
            waitpid(pid, &status, 0);

            if (WIFSIGNALED(status)) {
                printf("ndshell: process %d exited abnormally with signal %d: %s\n", pid, WTERMSIG(status), strsignal(WTERMSIG(status)));
            }
        } 
        
        else {
            // The child process has already exited
            if (WIFEXITED(status)) {
                printf("ndshell: process %d exited with status %d\n", pid, WEXITSTATUS(status));
            } 
            
            else if (WIFSIGNALED(status)) {
                printf("ndshell: process %d exited abnormally with signal %d: %s\n", pid, WTERMSIG(status), strsignal(WTERMSIG(status)));
            }
        }
    }
}

int main() {
    char input[100];
    char *args[10];
    int status;
    
    // Register signal handler for SIGINT
    struct sigaction sa;
    sa.sa_handler = handle_control_c;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    // Infinite loop receives input and runs functions
    while (1) {

        printf("ndshell> ");

        if (fgets(input, 100, stdin) == NULL) {
            // End-of-file or error
            printf("\n");
            break;
        }

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
        } 
        
        else if (strcmp(args[0], "exit") == 0) {
            // Exit the shell
            printf("Exiting shell immediately\n");
            break;
        } 
        
        else if (strcmp(args[0], "start") == 0) {
            // Start a new process with the given arguments
            start_process(&args[1]);
        } 
        
        else if (strcmp(args[0], "wait") == 0) {
            // Wait for any child process to exit
            wait_for_all_processes();
        } 
        
        else if (strcmp(args[0], "waitfor") == 0) {
            // Wait for a specific child process to exit
            if (num_args < 2) {
                printf("Usage: waitfor <pid>\n");
            } 
            
            else {
                pid_t pid = atoi(args[1]);
                wait_for_process(pid);
            }
        } 
        
        else if (strcmp(args[0], "run") == 0) {
            // Start a new process and wait for it to exit
            start_process(&args[1]);
            wait_for_process(last_pid);
        } 
        
        else if (strcmp(args[0], "kill") == 0) {
            // Kill a specific child process
            if (num_args < 2) {
                printf("Usage: kill <pid>\n");
            } 
            
            else {
                pid_t pid = atoi(args[1]);
                kill_process(pid);
            }
        } 
        
        else if (strcmp(args[0], "quit") == 0) {
            // Terminate all child processes and exit the shell
            quit();
        } 
        
        else if (strcmp(args[0], "bound") == 0) {
            // Run a time-bound process and kill it if it exceeds the time limit
            if (num_args < 3) {
                printf("Usage: bound <time limit> <command>\n");
            } 

            else {
                int time_limit = atoi(args[1]);
                bound(time_limit, &args[2]);
            }
        } 
        
        else {
            printf("ndshell: Error - unknown command: %s\n", args[0]);
        }
    }
}

