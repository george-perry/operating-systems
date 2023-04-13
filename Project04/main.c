/* main.c : Main file for redextract */

#include <stdio.h>
#include <stdlib.h>

/* for strdup due to C99 */
char * strdup(const char *s);

#include <string.h>

#include "pcap-read.h"
#include "pcap-process.h"
#include "ds_cv.h"


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


    Queue *packet_queue = createQueue(1024);
    pthread_t reader_threads[argc - 1];

    for (int i = 1; i < argc; i++) {

        ThreadArgs* thread_args = (ThreadArgs*) malloc(sizeof(ThreadArgs));
        struct FilePcapInfo     theInfo;

        theInfo.FileName = strdup(argv[i]);
        theInfo.EndianFlip = 0;
        theInfo.BytesRead = 0;
        theInfo.Packets = 0;
        theInfo.MaxPackets = 5;

        thread_args->theInfo = theInfo;
        thread_args->packet_queue = packet_queue;

        pthread_create(&reader_threads[i - 1], NULL, read_pcap_wrapper, (void*) thread_args);
    }


    pthread_t consumer_thread[1];

    for (int i = 1; i < 3; i++) {
        pthread_create(&consumer_thread[i-1], NULL, processPacket_wrapper, (void*) packet_queue);
    }


    // Wait for reader threads to complete
    for (int i = 0; i < argc - 1; i++) {
        pthread_join(reader_threads[i], NULL);
    }

    // Wait for consumer threads to complete
    for (int i = 0; i < 2; i++) {
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


    return 0;
}
