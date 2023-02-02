#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

// Help function tells user the correct usage
void help() {
    printf("Usage: ./bitflip File [-o OutputFile] [-maxsize MaxSize] [-bfr] [-r]\n");
    printf("\tFile: Required, the file to take in as the input\n");
    printf("\t-o: Optional, an override of the output file name\n");
    printf("\t-maxsize: Optional, an override in kB of the maximum file size allowed (integer > 0)\n");
    printf("\t-bfr: Optional, reverse the order of the bytes and bit-flip the bits in the file\n");
    printf("\t-r: Optional, reverse the order of the bytes in the file without bit-flipping\n");
}

int main(int argc, char *argv[]) {

    // Initialize variables for arguments
    int max_size = 25*1024;
    bool reverse_flip = false;
    bool reverse_no_flip = false;
    char *file_name = NULL;
    char *output_file_name = NULL;

    // Minimum of 2 arguments required
    if (argc < 2) {
        printf("Error: No file specified\n");
        exit(1);
    }

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        
        // If -help is entered, call the help function
        if (strcmp(argv[i], "-help") == 0) {
            help();
        } 

        // If -o is entered, check save valid output file name
        else if (strcmp(argv[i], "-o") == 0) {
            if (i+1 < argc) {
                output_file_name = argv[i+1];
            } 
            else {
                printf("Error: no output file name given\n");
                exit(1);
            }
            i++;
        } 
        
        // If -maxsize is entered, save valid size variable
        else if (strcmp(argv[i], "-maxsize") == 0) {
            if (i+1 < argc) {
                max_size = atoi(argv[i+1]);

                // Use strtol to check if the conversion to integer is valid
                char *endptr;
                long value = strtol(argv[i+1], &endptr, 10);

                // If the conversion stopped, or if max_size invalid, print error and exit
                if (*endptr != '\0'|| max_size <= 0) {
                    printf("Error: Max size must between an integer greater than 0\n");
                    exit(1);
                }
            } 
            else {
                printf("Error: missing size\n");
                return 1;
            }
            i++;
        } 
        
        // If -bfr is entered, set boolean flag to true
        else if (strcmp(argv[i], "-bfr") == 0) {
            reverse_flip = true;
        } 

        // If -r is entered, set boolean flag to true
        else if (strcmp(argv[i], "-r") == 0) {
            reverse_no_flip = true;
        } 
        
        // If the file name was not yet saved, save it now as it must be current argument (unless invalid args)
        else if (!file_name) {
            file_name = argv[i];
        } 
        
        // If there are invalid arguments, print error and exit
        else {
            printf("Error: invalid argument %s\n", argv[i]);
            exit(1);
        }
    }

    // Create a file pointer and check if the file is valid
    FILE* fp = fopen(file_name, "r");
    if (fp == NULL) {
        printf("Please retry: Invalid File\n");
        exit(1);
    }


    // Ensure the file is within the specified or default size limit
    fseek(fp, 0, SEEK_END);
    long int size = ftell(fp);

    if (size > max_size) {
        printf("Please retry - The size of the file is greater than %d kilobytes\n", max_size);
        fclose(fp);
        exit(1);
    }

    // Reset file pointer
    rewind(fp);


    // If -bfr and -r are entered, print error and exit
    if (reverse_flip && reverse_no_flip) {
        printf("Error: Cannot use -r and -bfr together\n");
        exit(1);
    }

    // Create a copy of the initial file name
    char *initial_name = strdup(file_name);

    // If no output file name was provided, create one based off -bfr / -r flags or lack thereof
    if (!output_file_name) {
        
        char *extension = NULL;

        if (reverse_flip) {
            extension = ".bfr";
        }
        else if (reverse_no_flip) {
            extension = ".r";
        }
        else {
            extension = ".bf";
        }

        output_file_name = strcat(file_name, extension);
    }

    // If the output file already exists, print error and exit
    if (fopen(output_file_name, "r")) {
        printf("Error: Output file %s already exists\n", output_file_name);
        exit(1);
    }
    
    // Open output file for writing
    FILE* ofp = fopen(output_file_name, "w");


    // Create a buffer of the file's size, and use fread to read in data
    char buffer[size];
    size_t bytes = fread(buffer, 1, size, fp);

    // If -bfr entered, reverse file and byte flip
    if (reverse_flip) {

        // Loop through file and use a temporary variable to reverse it
        int j = 0;
        for (int i = size - 1; i >= j; i--) {
            char temp = buffer[i];
            buffer[i] = buffer[j];
            buffer[j] = temp;
            j++;
        }

        // Flip each byte using a XOR with 0xff
        for (int i = 0; i < size; i++) {
            buffer[i] ^= 0xff;
        }
    }

    // If -r entered, just reverse the file
    else if (reverse_no_flip) {

        int j = 0;
        for (int i = size - 1; i >= j; i--) {
            char temp = buffer[i];
            buffer[i] = buffer[j];
            buffer[j] = temp;
            j++;
        }
    }

    // If no flags were entered, just byte flip the file
    else {
        for (int i = 0; i < size; i++) {
            buffer[i] ^= 0xff;
        }
    }

    // Rewind the input file and write reversed/flipped file to output
    rewind(fp);
    bytes = fwrite(buffer, 1, size, ofp);

    printf("Input: %s was %ld bytes\n", initial_name, size);
    printf("Output: %s was output successfully\n", output_file_name);

    // Close files, free memory, and return
    fclose(fp);
    fclose(ofp);
    free(initial_name);
    return 0;
}
