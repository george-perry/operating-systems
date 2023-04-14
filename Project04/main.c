/* main.c : Main file for redextract */

#include <stdio.h>
#include <stdlib.h>

/* for strdup due to C99 */
char * strdup(const char *s);

#include <string.h>

#include "pcap-read.h"
#include "pcap-process.h"
#include "queue.h"

#define MAX_FILES 10

int main (int argc, char *argv[])
{ 
    if(argc < 2)
    {
        printf("Usage: redextract FileX\n");
        printf("       redextract FileX\n");
        printf("  FileList        List of pcap files to process\n");
        printf("    or\n");
        printf("  FileName        Single file to process (if ending with .pcap)\n");
        printf("\n");
        printf("Optional Arguments:\n");
        /* You should handle this argument but make this a lower priority when 
           writing this code to handle this 
         */
        printf("  -threads N       Number of threads to use (2 to 8)\n");
        /* Note that you do not need to handle this argument in your code */
        printf("  -window  W       Window of bytes for partial matching (64 to 512)\n");
        printf("       If not specified, the optimal setting will be used\n");
        return -1;
    }

    printf("MAIN: Initializing the table for redundancy extraction\n");
    initializeProcessing(DEFAULT_TABLE_SIZE);
    printf("MAIN: Initializing the table for redundancy extraction ... done\n");

    /* Note that the code as provided below does only the following 
     *
     * - Reads in a single file twice
     * - Reads in the file one packet at a time
     * - Process the packets for redundancy extraction one packet at a time
     * - Displays the end results
     */

    // Initialize default # of consumer threads and window 
    int num_consumers = 7;
    int window = 0;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-threads") == 0) {
            // Parse the number of threads
            i++;
            if (i >= argc) {
                printf("Error: -threads option requires an argument\n");
                return -1;
            }
            num_consumers = atoi(argv[i]);
            if (num_consumers < 2 || num_consumers > 8) {
                printf("Error: -threads option requires an argument between 2 and 8\n");
                return -1;
            }
        } 
        
        else if (strcmp(argv[i], "-window") == 0) {
            // Parse the window size
            i++;
            if (i >= argc) {
                printf("Error: -window option requires an argument\n");
                return -1;
            }
            window = atoi(argv[i]);
            if (window < 64 || window > 512) {
                printf("Error: -window option requires an argument between 64 and 512\n");
                return -1;
            }
        } 
        
        else {
            printf("Error: unknown option '%s'\n", argv[i]);
            return -1;
        }
    }

    // Initialize vars to keep track of file/s
    char **file_names = malloc(MAX_FILES * sizeof(char*));
    char *file_name = NULL;
    int num_files = 1;

    // Parse file argument - if just 1, save
    if (strstr(argv[1], ".pcap") != NULL) {
        file_names[0] = argv[1];
    } 
    // Otherwise, open file with multiple file names, and parse
    else {

        FILE *fp = fopen(argv[1], "r");
        if (fp == NULL) {
            printf("Error: cannot open file %s\n", argv[1]);
            return -1;
        }

        // Count the number of files in the list
        num_files = 0;
        char buffer[256];
        while (fgets(buffer, 256, fp)) {
            if (strstr(buffer, ".pcap\n") != NULL) {
                num_files++;
            }
        }
        fclose(fp);

        // Populate array with file names
        fp = fopen(argv[1], "r");
        if (fp == NULL) {
            printf("Error: cannot open file %s\n", argv[1]);
            return -1;
        }

        int i = 0;
        while (fgets(buffer, 256, fp)) {
            if (strstr(buffer, ".pcap\n") != NULL) {
                file_names[i] = strdup(buffer);
                file_names[i][strlen(file_names[i]) - 1] = '\0'; // remove newline character
                i++;
            }
        }
        fclose(fp);
    }

    // Allocate memory for the reader threads and FilePcapInfo structs, create queue
    pthread_t *reader_threads;
    reader_threads = malloc(num_files * sizeof(pthread_t));
    Queue *packet_queue = createQueue(1024);

    // Iterate over total # of files
    for (int i = 0; i < num_files; i++) {

        // on each iteration, create new thread_args and theInfo structs
        ThreadArgs* thread_args = (ThreadArgs*) malloc(sizeof(ThreadArgs));
        struct FilePcapInfo     theInfo;

        // Save attributes to structs
        theInfo.FileName = strdup(file_names[i]);
        theInfo.EndianFlip = 0;
        theInfo.BytesRead = 0;
        theInfo.Packets = 0;
        theInfo.MaxPackets = 5;

        thread_args->theInfo = theInfo;
        thread_args->packet_queue = packet_queue;
        thread_args->num_files = num_files;

        // Create thread for each file, and pass args to readPcap wrapper
        pthread_create(&reader_threads[i], NULL, readPcapFile_producer, (void*) thread_args);
    }

    // Create specified # or default # of consumers
    pthread_t *consumer_thread = malloc(num_consumers * sizeof(pthread_t));

    for (int i = 0; i < num_consumers; i++) {
        pthread_create(&consumer_thread[i], NULL, dequeue, (void*) packet_queue);
    }

    // Wait for reader threads to complete
    for (int i = 0; i < num_files; i++) {
        pthread_join(reader_threads[i], NULL);
    }

    // Wait for consumer threads to complete
    for (int i = 0; i < num_consumers; i++) {
        pthread_join(consumer_thread[i], NULL);
    }

    printf("Summarizing the processed entries\n");
    tallyProcessing();

    /* Output the statistics */

    printf("Parsing of file %s complete\n", argv[1]);

    printf("  Total Packets Parsed:    %d\n", gPacketSeenCount);
    printf("  Total Bytes   Parsed:    %lu\n", (unsigned long) gPacketSeenBytes);
    printf("  Total Packets Duplicate: %d\n", gPacketHitCount);
    printf("  Total Bytes   Duplicate: %lu\n", (unsigned long) gPacketHitBytes);

    float fPct;
    fPct = (float) gPacketHitBytes / (float) gPacketSeenBytes * 100.0;

    printf("  Total Duplicate Percent: %6.2f%%\n", fPct);

    // Free all memeory
    deleteQueue(packet_queue);
    free(reader_threads);
    free(consumer_thread);

    free(file_names);

    return 0;
}
