#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>

char g_bKeepLooping = 1;
pthread_mutex_t g_mutex;

#define MAX_THREADS 5
#define BUFFER_SIZE 1024

struct ChimeThreadInfo {
    int nIndex;
    float fChimeInterval;
    char bIsValid;
    pthread_t ThreadID;
};

struct ChimeThreadInfo TheThreads[MAX_THREADS];

void *ThreadChime(void *pData) {
    struct ChimeThreadInfo *pThreadInfo;

    /* Which chime are we? */
    pThreadInfo = (struct ChimeThreadInfo *)pData;

    // Continue to print out the chimes on the specified intervals
    while (g_bKeepLooping) {
        sleep(pThreadInfo->fChimeInterval);
        printf("Ding - Chime %d with an interval of %.2f s!\n", pThreadInfo->nIndex, pThreadInfo->fChimeInterval);
    }
    return NULL;
}

void JoinChimeThread(int index) {
    // Join the threads
    pthread_join(TheThreads[index].ThreadID, NULL);
    printf("Join Complete for Chime %d!\n", index);
    TheThreads[index].bIsValid = 0;
}

void StartChimeThread(int index, float interval) {
    // Initialize thread and print attributes
    TheThreads[index].nIndex = index;
    TheThreads[index].fChimeInterval = interval;
    TheThreads[index].bIsValid = 1;
    pthread_create(&TheThreads[index].ThreadID, NULL, ThreadChime, &TheThreads[index]);
    printf("Starting thread %lu for chime %d, interval of %.2f s\n", TheThreads[index].ThreadID, index, interval);

}

int main(int argc, char *argv[]) {
    char szBuffer[BUFFER_SIZE];

    /* Set all of the thread information to be invalid (none allocated) */
    for (int j = 0; j < MAX_THREADS; j++) {
        TheThreads[j].bIsValid = 0;
    }

    // Initialize mutex
    pthread_mutex_init(&g_mutex, NULL);

    while (1) {
        /* Prompt and flush to stdout */
        printf("CHIME>");
        fflush(stdout);

        /* Wait for user input via fgets */
        fgets(szBuffer, BUFFER_SIZE, stdin);

        /* If the command is quit - join any active threads and finish up gracefully */
        if (strncmp(szBuffer, "exit", 4) == 0) {
            g_bKeepLooping = 0;
            // Loop through threads and join before exiting
            for (int j = 0; j < MAX_THREADS; j++) {
                if (TheThreads[j].bIsValid) {
                    printf("Joining Chime %d (Thread %lu)\n", j, TheThreads[j].ThreadID);
                    JoinChimeThread(j);
                }
            }
            printf("Exit chime program ...\n");
            break;
        }

        /* If the command is chime, the second argument is the chime number (integer) and the
           third number is the new interval (floating point). If necessary, start the thread
           as needed */
        int index;
        float interval;
        int result = sscanf(szBuffer, "chime %d %f", &index, &interval);

        // If 2 args given, continue on
        if (result == 2) {
            // If the input is invalid, print error
            if (index < 0 || index >= MAX_THREADS) {
                printf("Cannot adjust chime %d, out of range\n", index);
                continue;
            }
            // If chime already created
            if (TheThreads[index].bIsValid) {
                // Lock call the mutex and adjust interval, then free the mutex
                pthread_mutex_lock(&g_mutex);
                TheThreads[index].fChimeInterval = interval;
                pthread_mutex_unlock(&g_mutex);
                printf("Adjusting chime %d to have an interval of %.2f s\n", index, interval);
            } 
            // Otherwise, create new with specified index/interval
            else {
                StartChimeThread(index, interval);
            }
        } 
        // Otherwise, print error
        else {
            printf("Unknown command: %s", szBuffer);
        }
    }

    // Stop and join all active threads before exiting
    for (int i = 0; i < MAX_THREADS; i++) {
        if (TheThreads[i].bIsValid) {
            pthread_cancel(TheThreads[i].ThreadID);
            pthread_join(TheThreads[i].ThreadID, NULL);
        }
    }

    return 0;
}