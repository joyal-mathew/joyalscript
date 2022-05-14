#include "auxiliary.h"

#pragma once

typedef enum {
    Occupied,
    Empty,
} EntryType;

typedef struct {
    const char *key;
    u64 value;
    EntryType type;
} Entry;

typedef struct {
    Entry *map;
    u64 len;
} HashMap;

void hashmap_init(HashMap *hm);
void hashmap_deinit(HashMap *hm);
bool  hashmap_get(HashMap *hm, const char *key, u64 *value);
void hashmap_put(HashMap *hm, const char *key, u64 value);
