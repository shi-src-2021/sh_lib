#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include "sh_hash.h"
#include "sh_assert.h"

#ifndef SH_MALLOC
    #define SH_MALLOC   malloc
#endif

#ifndef SH_FREE
    #define SH_FREE     free
#endif

struct sh_hash_node {
    const char* key;
    uint32_t    key_len;
    void*       value;
    bool        is_exist;
};

struct sh_hash_handle {
    struct sh_hash_node* nodes;
    uint32_t node_count;
    uint32_t used;
};

static inline uint32_t jenkins_hash(const char *key, uint32_t key_len)
{
    SH_ASSERT(key);
    
    uint32_t i = 0;
    uint32_t hash = 0;

    while (i != key_len) {
        hash += key[i++];
        hash += hash << 10;
        hash ^= hash >> 6;
    }

    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;

    return hash;
}

static uint32_t sh_jen_hash(const char *key, uint32_t bucket_len)
{
    SH_ASSERT(key);

    return jenkins_hash(key, strlen(key)) % bucket_len;
}

static uint32_t sh_hash_get_next_index(sh_hash_t *hash, uint32_t index)
{
    SH_ASSERT(hash);

    if (index == hash->node_count - 1) {
        index = 0;
    } else {
        index++;
    }

    return index;
}

sh_hash_t* sh_hash_create(size_t size)
{
    sh_hash_t *hash = (sh_hash_t*)SH_MALLOC(sizeof(sh_hash_t));
    if (hash == NULL) {
        return NULL;
    }

    struct sh_hash_node *nodes = 
        (struct sh_hash_node*)SH_MALLOC(size * sizeof(struct sh_hash_node));
    if (nodes == NULL) {
        goto free_hash_t;
    }

    memset(nodes, 0, size * sizeof(struct sh_hash_node));

    hash->nodes = nodes;
    hash->node_count = size;
    hash->used = 0;
    
    return hash;

free_hash_t:
    SH_FREE(hash);
    return NULL;
}

void sh_hash_destroy(sh_hash_t *hash)
{
    if (hash == NULL) {
        return;
    }

    SH_FREE(hash->nodes);
    SH_FREE(hash);
}

int sh_hash_add(sh_hash_t *hash, const char *key, void *value)
{
    SH_ASSERT(hash);
    SH_ASSERT(key);

    uint32_t hashv = sh_jen_hash(key, hash->node_count);

    if (hash->used >= hash->node_count) {
        return -1;
    }
    
    while (hash->nodes[hashv].is_exist) {
        hashv = sh_hash_get_next_index(hash, hashv);
    }

    hash->nodes[hashv].key = key;
    hash->nodes[hashv].key_len = strlen(key);
    hash->nodes[hashv].value = value;
    hash->nodes[hashv].is_exist = true;

    hash->used++;
    return 0;
}

static int sh_hash_find_index(sh_hash_t *hash, const char *key, uint32_t *index)
{
    SH_ASSERT(hash);
    SH_ASSERT(key);

    uint32_t hashv = sh_jen_hash(key, hash->node_count);
    uint32_t key_len = strlen(key);

    for (uint32_t i = 0; hash->nodes[hashv].is_exist;
         i++, hashv = sh_hash_get_next_index(hash, hashv))
    {
        if (i >= hash->node_count) {
            break;
        }

        if (key_len != hash->nodes[hashv].key_len) {
            continue;
        }

        if (memcmp(hash->nodes[hashv].key, key, key_len) == 0) {
            if (index != NULL) {
                *index = hashv;
            }
            return 0;
        }
    }

    return -1;
}

int sh_hash_find(sh_hash_t *hash, const char *key, void **value)
{
    SH_ASSERT(hash);
    SH_ASSERT(key);

    uint32_t index = 0;

    if (!sh_hash_find_index(hash, key, &index)) {
        if (value != NULL) {
            *value = hash->nodes[index].value;
        }
        return 0;
    }

    return -1;
}

void sh_hash_clear(sh_hash_t *hash)
{
    SH_ASSERT(hash);

    memset(hash->nodes, 0, hash->node_count * sizeof(struct sh_hash_node));
    hash->used = 0;
}

void sh_hash_remove(sh_hash_t *hash, const char *key)
{
    SH_ASSERT(hash);
    SH_ASSERT(key);

    uint32_t index = 0;

    if (!sh_hash_find_index(hash, key, &index)) {
        memset(&hash->nodes[index], 0, sizeof(struct sh_hash_node));
        hash->used -= (hash->used > 0);
    }
}

bool sh_hash_is_key_exist(sh_hash_t *hash, const char *key)
{
    SH_ASSERT(hash);
    SH_ASSERT(key);

    return (sh_hash_find_index(hash, key, NULL) == 0);
}

int sh_hash_get_used_amount(sh_hash_t *hash)
{
    SH_ASSERT(hash);

    return hash->used;
}

