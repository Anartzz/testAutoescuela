#ifndef stack_h
#define stack_h

typedef struct stackNode {
    void* value;
    struct stackNode* next;    
} stackNode;

typedef struct Stack {
    stackNode* head;
    int length;
} Stack;

void stPush(Stack* stack, void* value);

void* stPop(Stack* stack);

void* stPeek(Stack* stack);

#endif
