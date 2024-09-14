#include <stdbool.h>

#include "sh_mem.h"
#include "tlsf.h"

static tlsf_t  tlsf = NULL;
static char mem_pool[MEM_POOL_SIZE];
static bool is_mem_created = false;

void* sh_malloc(size_t size)
{
    if (is_mem_created == false) {
        is_mem_created = true;
        tlsf = tlsf_create_with_pool(mem_pool, sizeof(mem_pool));
    }

    return tlsf_malloc(tlsf, size);
}

void* sh_realloc(void* ptr, size_t size)
{
    return tlsf_realloc(tlsf, ptr, size);
}

void sh_free(void* ptr)
{
    tlsf_free(tlsf, ptr);
}

int sh_get_free_size(void)
{
    if (is_mem_created == false) {
        is_mem_created = true;
        tlsf = tlsf_create_with_pool(mem_pool, sizeof(mem_pool));
    }
    
    return (int)tlsf_get_free_size(tlsf);
}

