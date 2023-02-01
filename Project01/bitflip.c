#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main(int argc, char* argv[]) {

    // Check to see if the # of CLA is correct
    if (argc != 3) {
        printf("Please retry: Invalid arguments\n");
        exit(1);
    }

    // Create a file pointer and check if the file is valid
    FILE* fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("Please retry: Invalid File\n");
        exit(1);
    }

    fclose(fp);
    return 0;
}
