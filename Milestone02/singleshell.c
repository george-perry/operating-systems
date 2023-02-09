#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

// Function used to handle Control-C
void handle_ctrlc(int theSIG) {
    printf("\nControl-C was pressed .. exiting\n");
    exit(0);
}

int main(void) {

    // Register a function with the signal handler to catch Control-C
    signal(SIGINT, handle_ctrlc);

    // Ask what to execute and save to input
    printf("Execute? ");
    char input[50];
    fgets(input, 50, stdin);

    // Remove newline so command in correct format
    input[strlen(input) - 1] = 0;

    // Split the input into the executable and arguments, use strtok to seperate delimeter
    char *myargs[50];
    char *token = strtok(input, " ");
    int i = 0;

    while (token != NULL) {
        myargs[i] = token;
        token = strtok(NULL, " ");
        i++;
    }

    myargs[i] = NULL;

    // Fork the process
    int rc = fork();

    if (rc == -1) {
        // If -1, print that there's an error in forking and exit
        perror("Error in forking process");
        exit(1);

    } else if (rc == 0) {
        // If 0, it's a child process, execute
        printf("Executing: %s\n", input);
        execvp(myargs[0], myargs);

        // Print and exit if error in execution
        perror("Error in executing the process");
        exit(1);

    } else {
        // Else, it's a parent process, wait and complete
        int wc = wait(NULL);
        printf("Execution complete!\n");
    }

    return 0;
}
