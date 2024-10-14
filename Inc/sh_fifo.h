#ifndef __SH_FIFO_H__
#define __SH_FIFO_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "sh_mem.h"

typedef struct sh_fifo {
    uint32_t in;
    uint32_t out;
    uint32_t esize;
    uint32_t size;
    void    *data;
} sh_fifo_t;

int sh_fifo_init(sh_fifo_t *fifo, void *data, uint32_t size, uint32_t esize);
sh_fifo_t* sh_fifo_create(uint32_t size, uint32_t esize);
void sh_fifo_destroy(sh_fifo_t *fifo);
uint32_t sh_fifo_get_used_size(sh_fifo_t *fifo);
uint32_t sh_fifo_get_unused_size(sh_fifo_t *fifo);
uint32_t sh_fifo_in(sh_fifo_t *fifo, void *buf, uint32_t size);
uint32_t sh_fifo_out(sh_fifo_t *fifo, void *buf, uint32_t size);

#ifdef __cplusplus
}   /* extern "C" */ 
#endif

#endif
