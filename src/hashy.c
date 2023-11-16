#define INITIAL_CAPACITY 32
#define MAX_LOAD_FACTOR 0.7
#include "hashy.h"

// Windows does not yet have C23
#ifdef _WIN32
#define strdup _strdup
#endif

static unsigned long
hash_string(const char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

HashTable *
hashy_init() {
    HashTable *table = malloc(sizeof(HashTable));
    if (!table) {
        return NULL;
    }

    table->capacity = INITIAL_CAPACITY;
    table->size = 0;
    table->entries = calloc(table->capacity, sizeof(HashEntry));
    if (!table->entries) {
        free(table);
        return NULL;
    }

    return table;
}

void
hashy_free(HashTable *table) {
    if (!table)
        return;

    for (size_t i = 0; i < table->capacity; i++) {
        free(table->entries[i].key);
    }
    free(table->entries);
    free(table);
}

static bool
hashy_resize(HashTable *table, size_t new_capacity) {
    HashEntry *old_entries = table->entries;
    size_t old_capacity = table->capacity;

    table->entries = calloc(new_capacity, sizeof(HashEntry));
    if (!table->entries) {
        table->entries = old_entries;
        return false;
    }
    table->capacity = new_capacity;
    table->size = 0;

    for (size_t i = 0; i < old_capacity; i++) {
        if (old_entries[i].key) {
            hashy_insert(table, old_entries[i].key, old_entries[i].value);
            free(old_entries[i].key);
        }
    }
    free(old_entries);
    return true;
}

/// RETS:
/// 0: insert success.
/// 1: duplicate key (key already exist).
/// 2: allocation error (resize).
int
hashy_insert(HashTable *table, const char *key, void *value) {
    if (table->size >= table->capacity * MAX_LOAD_FACTOR) {
        if (!hashy_resize(table, table->capacity * 2)) {
            return 2;
        }
    }

    unsigned long index = hash_string(key) % table->capacity;
    HashEntry *entry = &table->entries[index];

    while (entry->key != NULL) {
        if (strcmp(entry->key, key) == 0) {
            // Key already exists
            return 1;
        }
        index = (index + 1) % table->capacity;
        entry = &table->entries[index];
    }

    // Insert new entry
    entry->key = strdup(key);
    entry->value = value;
    table->size++;
    return 0;
}

/// RETS:
/// NULL: key not found / invalid.
/// ptr: value ptr.
void *
hashy_get(HashTable *table, const char *key) {
    unsigned long index = hash_string(key) % table->capacity;
    HashEntry *entry;

    for (size_t i = 0; i < table->capacity; i++) {
        entry = &table->entries[index];
        if (entry->key == NULL) {
            return NULL;
        }
        if (strcmp(entry->key, key) == 0) {
            return entry->value;
        }
        index = (index + 1) % table->capacity;
    }
    return NULL;
}

// https://github.com/senderista/hashtable-benchmarks/blob/master/src/main/java/set/int64/LPLongHashSet.java#L184
static void
shift_entries(HashTable *table, unsigned long start) {
    unsigned long dst = start;
    size_t shift = 1;

    while (true) {
        unsigned long src = (dst + shift) % table->capacity;
        if (table->entries[src].key == NULL) {
            break;
        }

        unsigned long preferred = hash_string(table->entries[src].key) % table->capacity;
        bool reachable;
        if (src <= dst) {
            reachable = (preferred <= dst && preferred > src);
        } else {
            reachable = (preferred <= dst || preferred > src);
        }

        if (reachable) {
            table->entries[dst] = table->entries[src];
            table->entries[src].key = NULL;
            table->entries[src].value = NULL;
            dst = (dst + shift) % table->capacity;
            shift = 1;
        } else {
            ++shift;
        }
    }
}

bool
hashy_remove(HashTable *table, const char *key) {
    unsigned long index = hash_string(key) % table->capacity;

    // Find the entry to remove
    for (size_t i = 0; i < table->capacity; i++) {
        HashEntry *entry = &table->entries[index];
        if (entry->key == NULL) {
            return false; // Key not found
        }
        if (strcmp(entry->key, key) == 0) {
            // Key found, remove the entry
            free(entry->key);
            entry->key = NULL;
            entry->value = NULL;
            table->size--;

            shift_entries(table, index);
            return true;
        }
        index = (index + 1) % table->capacity;
    }
    return false; // Key not found
}

HashTableIterator
hashy_iter(HashTable *table) {
    HashTableIterator iterator;
    iterator.table = table;
    iterator.current_index = 0;
    return iterator;
}

const char *
hashy_next(HashTableIterator *iterator, void **value) {
    while (iterator->current_index < iterator->table->capacity) {
        HashEntry *entry = &iterator->table->entries[iterator->current_index];
        iterator->current_index++;
        if (entry->key != NULL) {
            *value = entry->value;
            return entry->key;
        }
    }
    *value = NULL;
    return NULL;
}
