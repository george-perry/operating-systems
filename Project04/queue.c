#include "queue.h"
#include "pcap-process.h"

// Create the queue with default attributes
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

// Destroy mutex/cv and free the queue
void deleteQueue(Queue *q) {
    pthread_mutex_destroy(&q->lock);
    pthread_cond_destroy(&q->full);
    pthread_cond_destroy(&q->empty);
    free(q->buffer);
    free(q);
}

// Called while reading .pcap file - add a packet to the queue
int enqueue(Queue *q, struct Packet *packet) {

    pthread_mutex_lock(&q->lock);

    // Wait on CV
    while (q->count == q->capacity) {
        pthread_cond_wait(&q->full, &q->lock);
    }

    // Update circular buffer - serves for efficient memory usage
    q->rear = (q->rear + 1) % q->capacity;
    q->buffer[q->rear] = *packet;
    q->count++;

    pthread_cond_signal(&q->empty);
    pthread_mutex_unlock(&q->lock);
    return 0;
}

// Called by consumer threads to process a packet
void *dequeue(void *arg) {

    Queue *q = (Queue *)arg;

    // While there are still items in the queue
    while (q->KeepGoing || q->count > 0) {

        pthread_mutex_lock(&q->lock);

        // Wait on CV
        while (q->count == 0 && q->KeepGoing) {
            pthread_cond_wait(&q->empty, &q->lock);
        }

        // Update circular buffer and call processPacket while item in queue
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