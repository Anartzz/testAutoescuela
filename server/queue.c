#include <stdio.h>
#include <stdlib.h>
#include "include/queue.h"

Node* newNode(void* data) {
    Node* node = malloc(sizeof(Node));
    node->next = NULL;
    node->data = data;
    return node;
}

Queue* newQueue() {
    Queue* queue = malloc(sizeof(Queue));
    queue->head = NULL;
    queue->tail = NULL;
    queue->length = 0;
    return queue;
}

void* dequeue(Queue* queue) {
    if (!queue->head) {
        return NULL;
    }

    queue->length--;

    Node* head = queue->head;
    queue->head = head->next;
    void* data = head->data;

    free(head);

    if (queue->head == NULL) {
        queue->tail = NULL;
    }

    return data;
}

void enqueue(Queue* queue, void* data) {
    queue->length++;

    Node* node = newNode(data);

    if (queue->tail) {
        queue->tail->next = node;
    } else {
        queue->head = node;
    }
    queue->tail = node;
}

void* peek(Queue* queue) {
    return queue->head ? queue->head->data : NULL;
}

void queueFree(Queue* queue) {

    Node* head = queue->head;
    while (head) {
        Node* nextNode = head->next;
        free(head);
        head = nextNode;
    }

    free(queue);
}
