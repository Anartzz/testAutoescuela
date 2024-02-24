#ifndef queue_h
#define queue_h

typedef struct Node {
    void* data;
    struct Node* next;
} Node;

typedef struct Queue {
    Node* head;
    Node* tail;
    int length;

} Queue;

Queue* newQueue();

void* dequeue(Queue* queue);

void enqueue(Queue* queue, void* data);

void* peek(Queue* queue);

void queueFree(Queue* queue);

#endif
