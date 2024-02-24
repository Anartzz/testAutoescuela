
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/hashMap.h"

int primes[] = {13, 23, 53, 101, 199, 401, 797, 1619}; // ...

MapNode* newMapNode(char* key, void* value) {
    MapNode* node = malloc(sizeof(MapNode));
    node->key = key;
    node->value = value;
    node->next = NULL;
    return node;
}

Map* newMap() {
    Map* map = malloc(sizeof(Map));
    map->nodeNum = 0;
    map->capacity = primes[0];
    map->primesNum = 1;
    map->array = calloc(map->capacity, sizeof(MapNode*));
    return map;
}    

void mapFree(Map* map) {
    MapNode** array = map->array;
    for (int i = 0; i < map->capacity; i++) {
        MapNode* node = array[i];
        while (node) {
            MapNode* deleteNode = node;
            node = node->next;
            free(deleteNode);
        }
    }

    free(map->array);
    free(map);
}

unsigned int mapGetIndex(Map* map, char* key) {

    int num = 0;
    for (int i = 0; key[i] != 0; i++) {
        num += key[i];
    }

    return ((unsigned int) num * 238472389) % map->capacity;
}

void* mapGet(Map* map, char* key) {
    int index = mapGetIndex(map, key);
    MapNode* node = map->array[index];

    while (node && strcmp(node->key, key)) {
        node = node->next; 
    }

    if (!node) {
        return NULL;
    }

    return node->value;
}

void mapSetValue(Map* map, MapNode** array, MapNode* newNode) {
    int index = mapGetIndex(map, newNode->key);
    MapNode* node = array[index];
    if (node) {
        while (node->next) {
            node = node->next;
        }
        node->next = newNode;
    } else {
        array[index] = newNode;
    }
}

void mapResize(Map* map) {
    map->capacity = primes[map->primesNum++];
    MapNode** newArr = (MapNode**) calloc(map->capacity, sizeof(MapNode*));
    for (int i = 0; i < primes[map->primesNum - 2]; i++) {
        if (map->array[i]) {
            MapNode* node = map->array[i];
            do {
                MapNode* currNode = node;
                node = node->next;
                currNode->next = NULL;
                mapSetValue(map, newArr, currNode);
            } while (node);
        }
    }
    free(map->array);
    map->array = newArr;
}

void mapSet(Map* map, char* key, void* value) {
    map->nodeNum++;
    if ((map->nodeNum / map->capacity) > 0.75) {
        mapResize(map);
    }
    int index = mapGetIndex(map, key);
    MapNode* node = map->array[index];
    if (node) {
        while (node->next && strcmp(node->key, key)) {
            node = node->next;
        } 

        if (!strcmp(node->key, key)) {
            node->value = value;
            map->nodeNum--;
        } else {
            node->next = newMapNode(key, value);
        }

    } else {
        map->array[index] = newMapNode(key, value);
    }
}

void* mapRemove(Map* map, char* key) {
    map->nodeNum--;
    int index = mapGetIndex(map, key);
    MapNode* node = map->array[index];
    MapNode* prev = NULL;

    while (node && strcmp(node->key, key)) {
        prev = node;
        node = node->next; 
    }

    if (!node) {
        return NULL;
    }

    if (prev != NULL) {
        prev->next = node->next;
    } else {
        map->array[index] = node->next;
    }

    void* value = node->value;
    free(node);
    return value;

}

/*
#define eee "eee"

int main() { 
    Map* map = newMap();
    int num = 11;
    //int num2 = 141;
    mapSet(map, "pepe", (void*) &num);
    mapSet(map, "eee", (void*) 4);
    printf("%i\n", mapGet(map, eee));
    printf("%i\n", map->capacity);
    return 0;
}
*/
