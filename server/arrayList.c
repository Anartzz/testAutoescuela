#include <stdio.h>
#include <stdlib.h>
#include "include/arrayList.h"

ArrayList* newArrayList() {
    ArrayList* arrayList = malloc(sizeof(ArrayList));
    arrayList->array = calloc(1, sizeof(void*));
    arrayList->capacity = 1;
    arrayList->length = 0;
    return arrayList;
}

void freeArraList(ArrayList* arrayList) {
    free(arrayList->array);
    free(arrayList);
}

void alSet(ArrayList* list, int index, void* data) {
    if (index >= list->capacity) {
        do {
            list->capacity += 5;
        } while (index >= list->capacity);

        list->array = realloc(list->array, list->capacity * sizeof(void*));
    }     

    if (index >= list->length - 1) {
        list->length = index + 1;
    }

    list->array[index] = data;
}

void* alGet(ArrayList* list, int index) {
    if (list->length <= index) {
        return NULL; // not best
    }
    return list->array[index];
}

void alPush(ArrayList* list, void* data) {
    if (list->length >= list->capacity) {
        list->capacity += 5;
        list->array = realloc(list->array, list->capacity * sizeof(void*));
    }
    list->array[list->length] = data;
    list->length++;
}

void* alPop(ArrayList* list) {
    if (list->length <= 0) {
        return 0;
    }
    list->length--;
    void* data = list->array[list->length];
    list->array[list->length] = 0;
    return data;
}

void* alPeek(ArrayList* list) {
    if (list->length <= 0) {
        return 0;
    }
    return list->array[list->length - 1];
}

