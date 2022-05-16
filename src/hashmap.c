#include "hashmap.h"
#include <stdlib.h>
#include <string.h>

#define HASHMAP_SIZE 2
#define OFFSET_BASIS 14695981039346656037UL
#define PRIME 1099511628211

u64 hash(const char *s) {
    u64 hash = OFFSET_BASIS;

    for (; *s; ++s) {
        hash ^= *s;
        hash *= PRIME;
    }

    return hash;
}

void hashmap_init_len(HashMap *hm, u64 len) {
    hm->map = allocate(len, sizeof (Entry));
    hm->len = len;

    for (u64 i = 0; i < hm->len; ++i) {
        hm->map[i].token_type = et_Empty;
    }
}

void hashmap_init(HashMap *hm) {
    hashmap_init_len(hm, HASHMAP_SIZE);
}

void hashmap_deinit(HashMap *hm) {
    deallocate(hm->map);
}

void hashmap_rehash(HashMap *hm) {
    HashMap new;

    hashmap_init_len(&new, hm->len * 2 + 1);

    for (u64 i = 0; i < hm->len; ++i) {
        Entry entry = hm->map[i];

        if (entry.token_type == et_Occupied) {
            hashmap_put(&new, entry.key, entry.value);
        }
    }

    hashmap_deinit(hm);

    hm->map = new.map;
    hm->len = new.len;
}

RESULT hashmap_get(HashMap *hm, const char *key, u64 *value) {
    u64 hash_val = hash(key) % hm->len;
    u64 init_val = hash_val;

    for (;;) {
        Entry entry = hm->map[hash_val];

        if (entry.token_type == et_Occupied) {
            if (strcmp(entry.key, key) == 0) {
                *value = entry.value;
                return FALSE;
            }
        }
        else {
            return TRUE;
        }

        hash_val += 1;
        hash_val %= hm->len;

        if (hash_val == init_val) {
            return TRUE;
        }
    }
}

void hashmap_put(HashMap *hm, const char *key, u64 value) {
    u64 hash_val = hash(key) % hm->len;
    u64 init_val = hash_val;

    for (;;) {
        Entry *entry = &hm->map[hash_val];

        if (entry->token_type == et_Occupied) {
            if (strcmp(entry->key, key) == 0) {
                entry->value = value;
                return;
            }
        }
        else {
            entry->key = key;
            entry->value = value;
            entry->token_type = et_Occupied;
            return;
        }

        ++hash_val;
        hash_val %= hm->len;

        if (hash_val == init_val) {
            hashmap_rehash(hm);
            hashmap_put(hm, key, value);
            return;
        }
    }
}
