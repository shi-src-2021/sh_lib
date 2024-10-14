#include <string.h>

#include "sh_assert.h"
#include "sh_fifo.h"
#include "sh_lib.h"

#ifndef SH_MALLOC
    #define SH_MALLOC   malloc
#endif

#ifndef SH_FREE
    #define SH_FREE     free
#endif

int sh_fifo_init(sh_fifo_t *fifo, void *data, uint32_t size, uint32_t esize)
{
    SH_ASSERT(fifo);
    SH_ASSERT(data);

    fifo->in    = 0;
    fifo->out   = 0;
    fifo->data  = data;
    fifo->size  = size;
    fifo->esize = esize;

    return 0;
}

sh_fifo_t* sh_fifo_create(uint32_t size, uint32_t esize)
{
    sh_fifo_t *fifo = SH_MALLOC(sizeof(sh_fifo_t));
    if (fifo == NULL) {
        return NULL;
    }

    void *data = SH_MALLOC(size * esize);
    if (data == NULL) {
        SH_FREE(fifo);
        return NULL;
    }

    sh_fifo_init(fifo, data, size, esize);

    return fifo;
}

void sh_fifo_destroy(sh_fifo_t *fifo)
{
    if (fifo == NULL) {
        return;
    }

    if (fifo->data) {
        SH_FREE(fifo->data);
    }
    SH_FREE(fifo);
}

uint32_t sh_fifo_get_used_size(sh_fifo_t *fifo)
{
    SH_ASSERT(fifo);

    return (fifo->in + fifo->size - fifo->out) % fifo->size;
}

uint32_t sh_fifo_get_unused_size(sh_fifo_t *fifo)
{
    SH_ASSERT(fifo);

    return fifo->size - sh_fifo_get_used_size(fifo) - 1;
}

uint32_t sh_fifo_in(sh_fifo_t *fifo, void *buf, uint32_t size)
{
    SH_ASSERT(fifo);
    SH_ASSERT(buf);

    uint32_t esize = fifo->esize;

    uint32_t unused_size = sh_fifo_get_unused_size(fifo);
    if (size > unused_size) {
        size = unused_size;
    }

    uint32_t _len = fifo->size - (fifo->in % fifo->size);
    _len = MIN(_len, size);

    memcpy((uint8_t *)fifo->data + fifo->in * esize, buf, _len * esize);
    memcpy(fifo->data, (uint8_t *)buf + _len * esize, (size - _len) * esize);

    fifo->in = (fifo->in + size) % fifo->size;

    return size;
}

uint32_t sh_fifo_out_peek(sh_fifo_t *fifo, void *buf, uint32_t size)
{
    SH_ASSERT(fifo);
    SH_ASSERT(buf);

    uint32_t esize = fifo->esize;

    uint32_t used_size = sh_fifo_get_used_size(fifo);
    if (size > used_size) {
        size = used_size;
    }

    uint32_t _len = fifo->size - (fifo->out % fifo->size);
    _len = MIN(_len, size);

    memcpy(buf, (uint8_t *)fifo->data + fifo->out * esize, _len * esize);
    memcpy((uint8_t *)buf + _len * esize, fifo->data, (size - _len) * esize);

    return size;
}

uint32_t sh_fifo_out(sh_fifo_t *fifo, void *buf, uint32_t size)
{
    uint32_t len = sh_fifo_out_peek(fifo, buf, size);

    fifo->out = (fifo->out + len) % fifo->size;

    return len;
}







