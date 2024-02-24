#ifndef arrayList_h
#define arrayList_h

typedef struct {
    void** array;
    int length;
    int capacity;
} ArrayList;

ArrayList* newArrayList();

void freeArraList(ArrayList* arrayList);

void alSet(ArrayList* list, int index, void* data);

void* alGet(ArrayList* list, int index);

void alPush(ArrayList* list, void* data);

void* alPop(ArrayList* list);

void* alPeek(ArrayList* list);

#endif
