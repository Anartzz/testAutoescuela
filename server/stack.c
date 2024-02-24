#include <stdio.h>
#include <stdlib.h>
#include "./include/stack.h"

void stPush(Stack* stack, void* value) {

    stack->length++;
    stackNode* node = malloc(sizeof(stackNode));
    node->value = value;
    node->next = stack->head;
    stack->head = node;
}

void* stPop(Stack* stack) {

    if (!stack->head) {
        return NULL;
    }

    stack->length--;
    stackNode* head = stack->head; 
    stack->head = head->next;
    void* value = head->value;

    free(head);
    return value;
}

void* stPeek(Stack* stack) {
    return stack->head ? stack->head->value : NULL;
}
