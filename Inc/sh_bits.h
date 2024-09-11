#ifndef __SH_BITS_H__
#define __SH_BITS_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t sh_bits_get(uint32_t data, uint8_t start_bit, size_t bits);
uint32_t sh_bits_set(uint32_t data, uint8_t start_bit, size_t bits, uint32_t set);

#ifdef __cplusplus
}
#endif

#endif

