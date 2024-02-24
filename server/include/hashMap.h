#ifndef hashMap_h
#define hashMap_h

typedef struct MapNode {
    char* key;
    void* value;
    struct MapNode* next;
} MapNode;

MapNode* newMapNode(char* key, void* value);

typedef struct {
    int capacity;
    int nodeNum;
    int primesNum;
    MapNode** array;
} Map;

Map* newMap();

void mapFree(Map* map);

void* mapGet(Map* map, char* key);

void mapSet(Map* map, char* key, void* value);

void* mapRemove(Map* map, char* key);

#endif

