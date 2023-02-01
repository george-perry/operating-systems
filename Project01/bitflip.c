#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void help() {
    printf("Usage: ./bitflip File [-o OutputFile] [-maxsize MaxSize] [-bfr] [-r]\n");
    printf("\tFile: Required, the file to take in as the input\n");
    printf("\t-o: Optional, an override of the output file name\n");
    printf("\t-maxsize: Optional, an override of the maximum file size allowed for input\n");
    printf("\t-bfr: Optional, reverse the order of the bytes and bit-flip the bits in the file\n");
    printf("\t-r: Optional, reverse the order of the bytes in the file without bit-flipping\n");
}

int main(int argc, char *argv[]) {

    int max_size = 0;
    bool reverse_flip = false;
    bool reverse_no_flip = false;
    char *file_name = NULL;
    char *output_file_name = NULL;

    if (argc < 2) {
        printf("Error: No file specified\n");
        exit(1);
    }

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        
        if (strcmp(argv[i], "-help") == 0) {
            help();
        } 

        else if (strcmp(argv[i], "-o") == 0) {
            if (i+1 < argc) {
                output_file_name = argv[i+1];
            } 
            
            else {
                printf("Error: no output file name given\n");
                exit(1);
            }
        } 
        
        else if (strcmp(argv[i], "-maxsize") == 0) {
            if (i+1 < argc) {
                max_size = atoi(argv[i+1]);
            } 
            
            else {
                printf("Error: missing size\n");
            return 1;
            }
        } 
        
        else if (strcmp(argv[i], "-bfr") == 0) {
            reverse_flip = true;
        } 

        else if (strcmp(argv[i], "-r") == 0) {
            reverse_no_flip = true;
        } 
        
        else if (!file_name) {
            file_name = argv[i];
        } 
        
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

    if (reverse_flip && reverse_no_flip) {
        printf("Error: Cannot use -r and -bfr together\n");
        exit(1);
    }

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

    if (fopen(output_file_name, "r")) {
        printf("Error: Output file %s already exists\n", output_file_name);
        exit(1);
    }

    fclose(fp);
    return 0;
}
