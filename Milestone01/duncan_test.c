#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>




int main(int argc, char *argv[]) {
    FILE *input_file;
    char outword[100];
    char *filename = argv[1];
    char *byte_code = argv[2];
    char buffer[100];
    int count = 0;
    
    fprintf(stderr, "%s\n", byte_code);

    input_file = fopen(filename, "r");

    fread(buffer, 1, 1024, input_file);

    printf("%s\n", buffer);

    //for(int i = 0; i < strlen(buffer); i++){
    //    printf("%02x\n", buffer[i]);
    //}
    for(int i = 0; i<strlen(buffer); i++){
        sprintf(outword+i*3, "%02X ", buffer[i]);
    }

    printf("%s\n", outword);

    fclose(input_file);
}
/*
int main(void){
    char word[17], outword[33];//17:16+1, 33:16*2+1
    int i, len;

    printf("Intro word:");
    fgets(word, sizeof(word), stdin);
    //len = strlen(word);
    //if(word[len-1]=='\n')
    //    word[--len] = '\0';

    for(i = 0; i<strlen(word); i++){
        sprintf(outword+i*3, "%02X ", word[i]);
    }
    printf("%s\n", outword);
    return 0;
}*/