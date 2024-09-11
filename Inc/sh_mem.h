#ifndef __SH_MEM_H__
#define __SH_MEM_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MEM_POOL_SIZE   (20 * 1024)

void* sh_malloc(size_t size);
void* sh_realloc(void* ptr, size_t size);
void sh_free(void* ptr);
int sh_get_free_size(void);

#ifdef __cplusplus
}   /* extern "C" */ 
#endif

#endif

