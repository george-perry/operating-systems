#include <stdlib.h>

#ifndef __DS_CV_H
#define __DS_CV_H


#include <pthread.h>
#include "packet.h"
#include "pcap-read.h"

typedef struct {
    struct Packet *buffer;
    size_t capacity;
    size_t front;
    size_t rear;
    size_t count;
    pthread_mutex_t lock;
    pthread_cond_t full;
    pthread_cond_t empty;
} Queue;

typedef struct {
    struct FilePcapInfo theInfo;
    Queue* packet_queue;
} ThreadArgs;


Queue *createQueue(size_t capacity);
void deleteQueue(Queue *q);
int enqueue(Queue *q, struct Packet *packet);
struct Packet *dequeue(Queue *q);

#endif