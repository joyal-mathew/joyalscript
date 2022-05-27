#pragma once

#include "auxiliary.h"

typedef enum PACKED {
    et_Occupied,
    et_Empty,
} EntryType;

typedef struct {
    const char *key;
    u64 value;
    EntryType token_type;
} Entry;

typedef struct {
    Entry *map;
    u64 len;
} HashMap;

void hashmap_init(HashMap *hm);
void hashmap_deinit(HashMap *hm);
RESULT hashmap_get(HashMap *hm, const char *key, u64 *value);
void hashmap_put(HashMap *hm, const char *key, u64 value);
