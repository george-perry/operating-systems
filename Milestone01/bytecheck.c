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

    // Use strtol to save the input byte value to search for
    unsigned char searchByte = strtoul(argv[2], NULL, 16);

    // Verify that the file size is under the max size
    fseek(fp, 0, SEEK_END);
    long int size = ftell(fp);

    if (size > 25*1024) {
        printf("Please retry - The size of the file is greater than 25 kilobytes\n");
        fclose(fp);
        exit(1);
    }
    
    // Reset file pointer
    rewind(fp);

    // Initialize buffer
    char buffer[1024];

    // Count the number of occurrences of the byte value
    size_t bytes;
    int count = 0;

    // Now increment count for every occurrence of the input value
    while ((bytes = fread(buffer, 1, 1024, fp)) > 0) {
        for (int i = 0; i < bytes; i++) {
            if (buffer[i] == searchByte) {
                count++;
            }
        }
    }

    // Print out the final count
    printf("%d\n", count);

    // Close the file
    fclose(fp);
    return 0;
}
