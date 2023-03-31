/* Main.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>

#include "byteblock.h"

/* Up to 25 ms of delay on the producer */
#define DELAY_PRODUCER         0.024
#define RAND_DELAY_PRODUCER     0.001

/* Up to 50 ms of delay on the consumer */
#define DELAY_CONSUMER          0.024
#define RAND_DELAY_CONSUMER     0.001

/* How big should the block be to generate? */
#define BLOCK_GENERATE_SIZE     4096

/* How big can the stack get? */
#define STACK_MAX_SIZE          10

/* Use condition variables? */
//  Uncomment to enable this

#define USE_CONDITION_VARS

extern char* strdup(const char*);


/* The global queue */
pthread_mutex_t     StackLock;
struct ByteBlock *  StackItems[STACK_MAX_SIZE];
int                 StackSize = 0;
char                KeepGoing = 1;

int                 CountFound = 0;
pthread_mutex_t     FoundLock;

int                 CountDone = 0;
pthread_mutex_t     DoneLock;

int                 CountExpected = 0;


// Init mutex CV
pthread_cond_t     StackCV;
pthread_cond_t     StackCV_Pop;

struct ThreadDataProduce
{
    int Iterations;
    int ThreadID;
};

struct ThreadDataConsume
{
    int ThreadID;
    char * SearchString;
};


char stack_ts_cv_push (struct ByteBlock * pBlock)
{
    /* Condition variable version */
    /* Your code goes here! */

    // While the stack is full, wait on the CV

    pthread_mutex_lock(&StackLock);
    while (StackSize >= STACK_MAX_SIZE) {
        pthread_cond_wait(&StackCV, &StackLock);
    }

    // When signaled, add to stack and broadcast to CV_Pop

    StackItems[StackSize] = pBlock;
    StackSize++;

    pthread_cond_broadcast(&StackCV_Pop);
    pthread_mutex_unlock(&StackLock);

    return 1;
}

char stack_ts_push (struct ByteBlock * pBlock)
{
    pthread_mutex_lock(&StackLock);

    if(StackSize < STACK_MAX_SIZE)
    {
        StackItems[StackSize] = pBlock;
        StackSize++;        
        pthread_mutex_unlock(&StackLock);    
        return 1;
    }
    else 
    {
        pthread_mutex_unlock(&StackLock);    
        return 0;
    }
}

struct ByteBlock * stack_ts_cv_pop ()
{

    // While stack is empty and iterations unfinished, wait on CV_Pop
    pthread_mutex_lock(&StackLock);

    while (StackSize == 0 && CountDone < CountExpected) {
        pthread_cond_wait(&StackCV_Pop, &StackLock);
    }

    // If the stack isn't empty, remove item and signal to CV to push new

    if (StackSize > 0) {

        struct ByteBlock *pBlock = StackItems[--StackSize];

        pthread_mutex_unlock(&StackLock);
        pthread_cond_signal(&StackCV);

        return pBlock;

    } else {
        pthread_mutex_unlock(&StackLock);
        return NULL;
    }

}

struct ByteBlock * stack_ts_pop ()
{
    struct ByteBlock * pBlock;

    pthread_mutex_lock(&StackLock);

    if(StackSize > 0)
    {
        pBlock = StackItems[StackSize-1];
        StackSize--;
        pthread_mutex_unlock(&StackLock);    
        return pBlock;
    }
    else 
    {
        pthread_mutex_unlock(&StackLock);    
        return NULL;
    }
}

void * thread_producer (void * pData)
{
    int     IterationsToGo;
    int     nRandom;
    struct ByteBlock *  pBlock;
    int     ThreadID;

    struct ThreadDataProduce * pThreadData;

    pThreadData = (struct ThreadDataProduce *) pData;

    IterationsToGo = pThreadData->Iterations;
    ThreadID = pThreadData->ThreadID;

    /* Copied - get rid of the malloc'd allocation */
    free(pThreadData);

    while(KeepGoing)
    {
        printf("Thread %d - Iterations To Go: %d\n", ThreadID, IterationsToGo);

        if(IterationsToGo <= 0)
        {
            break;
        }        

        /******** COMPUTE BLOCK *******
         *   Do the thing 
         */

        pBlock = createBlock(BLOCK_GENERATE_SIZE);        

        for(int j=0; j<BLOCK_GENERATE_SIZE; j++)
        {
            nRandom = rand() % 3;

            if(nRandom == 0)
            {
                // Pick space
                pBlock->pData[j] = ' ';
            }
            else 
            {
                // Pick a letter
                nRandom = rand() % 90;
                pBlock->pData[j] = ' ' + (char) nRandom;                
            }                        
        }

        /* Mimic this being a really intense computation */
        sleep(DELAY_PRODUCER + ((float) rand() / (float) RAND_MAX) * RAND_DELAY_PRODUCER);

#ifndef USE_CONDITION_VARS
        /* Keep trying until we succeed! */
        while(!stack_ts_push(pBlock))
        {
            continue;
        }
#endif 

#ifdef USE_CONDITION_VARS
        /* Keep trying until we succeed! */
        while(!stack_ts_cv_push(pBlock))
        {
            continue;
        }
#endif

        IterationsToGo--;
    }

   printf("Producer thread %d is done!\n", ThreadID);
    return NULL;
}


void * thread_consumer (void * pData)
{
    struct ThreadDataConsume * pThreadData;
    char * SearchString;
    struct ByteBlock * pBlock;
    int     ThreadID;

    pThreadData = (struct ThreadDataConsume *) pData;

    ThreadID = pThreadData->ThreadID;
    SearchString = (char *) pThreadData->SearchString;

    /* Copied - get rid of the malloc'd allocation */
    free(pThreadData);    

    while(KeepGoing)
    {
        pthread_mutex_lock(&DoneLock);
        if(CountDone == CountExpected)
        {
            pthread_mutex_unlock(&DoneLock);

            // Broadcast to thread waiting to pop when done
            pthread_cond_broadcast(&StackCV_Pop);
            break;
        }

        pthread_mutex_unlock(&DoneLock);
 
        /******** COMPUTE BLOCK *******
         *   Do the thing 
         */

#ifndef USE_CONDITION_VARS
        pBlock = stack_ts_pop();   
#endif 

#ifdef USE_CONDITION_VARS 
        pBlock = stack_ts_cv_pop();
#endif

        if(pBlock != NULL)
        {
            printf("Thread %d - Operating on a Block\n", ThreadID);

            /* Search the block to see how many times the requested string appears */
            for(int j=0; j<pBlock->nSize - strlen(SearchString); j++)
            {
                if(memcmp(pBlock->pData+j, SearchString, strlen(SearchString)) == 0)
                {
                    pthread_mutex_lock(&FoundLock);
                    CountFound++;
                    pthread_mutex_unlock(&FoundLock);
                }
            }

            free(pBlock);

            /* Mimic this being a really intense computation */
            sleep(DELAY_CONSUMER + ((float) rand() / (float) RAND_MAX) * RAND_DELAY_CONSUMER);

            /* Adjust the completed count */
            pthread_mutex_lock(&DoneLock);
            CountDone++;
            pthread_mutex_unlock(&DoneLock);
        }     
    }

    printf("Consumer thread %d is done!\n", ThreadID);
    return NULL;
}

int main (int argc, char *argv[])
{    
    int     nThreadsProducers;
    int     nThreadsConsumers;   
    int     nIterations;
    int     j;

    pthread_mutex_init(&StackLock, 0);
    pthread_mutex_init(&FoundLock, 0);
    pthread_mutex_init(&DoneLock, 0);

    // Initialize CV 

    pthread_cond_init(&StackCV, 0);
    pthread_cond_init(&StackCV_Pop, 0);

    if(argc < 4)
    {
        printf("Usage: pcm4 Producers Consumers Iterations\n");
        printf("  Producers:  Number of producer threads\n");
        printf("  Consumers:  Number of consumer threads\n");
        printf("  Iterations: Number of iterations for the producers\n");
        return -1;
    }

    // Measure start time here!

    struct timeval start, end;
    gettimeofday(&start, NULL);

    // Check input

    if (*argv[1] == '\0' || !isdigit(*argv[1]) || *argv[2] == '\0' || !isdigit(*argv[2]) || *argv[3] == '\0' || !isdigit(*argv[3])) {
        printf("Invalid input - Arguments must be positive integerss\n");
        exit(1);
    }

    nThreadsProducers = atoi(argv[1]);
    nThreadsConsumers = atoi(argv[2]);
    nIterations = atoi(argv[3]);

    // Ensure input is positive
    
    if (nThreadsProducers <= 0 || nThreadsConsumers <= 0 || nIterations <= 0) {
        printf("Invalid input - values must be greater than 0\n");
        exit(1);
    }

    pthread_t *     pThreadProducers;
    pthread_t *     pThreadConsumers;

    /* Allocate space for tracking the threads */
    pThreadProducers = (pthread_t *) malloc(sizeof(pthread_t *) * nThreadsProducers); 
    pThreadConsumers = (pthread_t *) malloc(sizeof(pthread_t *) * nThreadsConsumers); 

    CountExpected = nThreadsProducers * nIterations;

    /* Start up our producer threads */
    for(j=0; j<nThreadsProducers; j++)
    {
        struct ThreadDataProduce  *    pThreadData;

        pThreadData = (struct ThreadDataProduce *) malloc(sizeof(struct ThreadDataProduce));
        pThreadData->Iterations = nIterations;
        pThreadData->ThreadID = j;

        pthread_create(pThreadProducers+j, 0, thread_producer, pThreadData);
    }

    /* Start up our consumer threads */
    for(j=0; j<nThreadsConsumers; j++)
    {
        struct ThreadDataConsume  *    pThreadData;

        pThreadData = (struct ThreadDataConsume *) malloc(sizeof(struct ThreadDataConsume));
        pThreadData->SearchString = strdup("the");
        pThreadData->ThreadID = j;

        pthread_create(pThreadConsumers+j, 0, thread_consumer, pThreadData);
    }

    /* Loop until the consumers are done */
    for(j=0; j<nThreadsConsumers; j++)
    {
        pthread_join(pThreadConsumers[j], NULL);
    }

    //  Output the total runtime in an appropriate unit

    gettimeofday(&end, NULL);
    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    printf("Elapsed time: %.6f seconds\n", elapsed_time);

    printf("Drumroll please .... %d occurrences of `the'\n", CountFound);


    return 0;
}

