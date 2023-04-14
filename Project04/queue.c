#include "queue.h"
#include "pcap-process.h"

Queue *createQueue(size_t capacity) {
    Queue *q = (Queue *)malloc(sizeof(Queue));
    q->buffer = (struct Packet *)malloc(sizeof(struct Packet) * capacity);
    q->capacity = capacity;
    q->front = 0;
    q->rear = -1;
    q->count = 0;
    q->KeepGoing = 1;
    q->total_packets = 10000;
    q->curr_total = 0;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->full, NULL);
    pthread_cond_init(&q->empty, NULL);
    return q;
}

void deleteQueue(Queue *q) {
    pthread_mutex_destroy(&q->lock);
    pthread_cond_destroy(&q->full);
    pthread_cond_destroy(&q->empty);
    free(q->buffer);
    free(q);
}

int enqueue(Queue *q, struct Packet *packet) {

    pthread_mutex_lock(&q->lock);
    while (q->count == q->capacity) {
        pthread_cond_wait(&q->full, &q->lock);
    }

    q->rear = (q->rear + 1) % q->capacity;
    q->buffer[q->rear] = *packet;
    q->count++;

    pthread_cond_signal(&q->empty);
    pthread_mutex_unlock(&q->lock);
    return 0;
}

void *dequeue(void *arg) {

    Queue *q = (Queue *)arg;

    while (q->KeepGoing || q->count > 0) {

        pthread_mutex_lock(&q->lock);
        while (q->count == 0 && q->KeepGoing) {
            pthread_cond_wait(&q->empty, &q->lock);
        }

        if (q->count > 0) {
            struct Packet *packet = &q->buffer[q->front];
            q->front = (q->front + 1) % q->capacity;
            q->count--;

            pthread_cond_signal(&q->full);
            pthread_mutex_unlock(&q->lock);

            // Perform redundancy computation on packet here
            processPacket(packet);

        } else {
            pthread_mutex_unlock(&q->lock);
        }
    }

    return NULL;

}