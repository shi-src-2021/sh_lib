#ifndef __SH_HASH_H__
#define __SH_HASH_H__

#include <stdbool.h>

#include "sh_mem.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SH_MALLOC   sh_malloc
#define SH_FREE     sh_free

typedef struct sh_hash_handle sh_hash_t;

sh_hash_t* sh_hash_create(size_t size);
void sh_hash_destroy(sh_hash_t *hash);
int sh_hash_add(sh_hash_t *hash, const char *key, void *value);
int sh_hash_find(sh_hash_t *hash, const char *key, void **value);
void sh_hash_clear(sh_hash_t *hash);
void sh_hash_remove(sh_hash_t *hash, const char *key);
bool sh_hash_is_key_exist(sh_hash_t *hash, const char *key);
int sh_hash_get_used_amount(sh_hash_t *hash);

#ifdef __cplusplus
}   /* extern "C" */ 
#endif

#endif
