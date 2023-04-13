#include "ds_cv.h"

Queue *createQueue(size_t capacity) {
    Queue *q = (Queue *)malloc(sizeof(Queue));
    q->buffer = (struct Packet *)malloc(sizeof(struct Packet) * capacity);
    q->capacity = capacity;
    q->front = 0;
    q->rear = -1;
    q->count = 0;
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
    pthread_mutex_unlock(&q->lock);
    pthread_cond_signal(&q->empty);
    return 0;
}

struct Packet *dequeue(Queue *q) {
    pthread_mutex_lock(&q->lock);
    while (q->count == 0) {
        pthread_cond_wait(&q->empty, &q->lock);
    }
    struct Packet *packet = &q->buffer[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->count--;
    pthread_mutex_unlock(&q->lock);
    pthread_cond_signal(&q->full);
    return packet;
}
