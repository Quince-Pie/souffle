#ifndef HASHY_H
#define HASHY_H

#include <stdlib.h>
#include <string.h>

typedef struct {
    char *key;
    void *value;
} HashEntry;

typedef struct {
    HashEntry *entries;
    size_t capacity;
    size_t size;
} HashTable;

HashTable *
hashy_init();

void
hashy_free(HashTable *table);

int
hashy_insert(HashTable *table, const char *key, void *value);

void *
hashy_get(HashTable *table, const char *key);

typedef struct HashTableIterator {
    HashTable *table;
    size_t current_index;
} HashTableIterator;

HashTableIterator
hashy_iter(HashTable *table);
const char *
hashy_next(HashTableIterator *iterator, void **value);

#endif // HASHY_H
