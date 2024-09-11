#include "sh_bits.h"

static uint32_t sh_bits_set_bit(uint32_t data, uint8_t bit, char value)
{
    value = !!value;
    data &= ~(1ull << bit);
    data |= (value << bit);

    return data;
}

uint32_t sh_bits_get(uint32_t data, uint8_t start_bit, size_t bits)
{
    uint32_t temp = data >> start_bit;
    temp &= ((1ull << bits) - 1);

    return temp;
}

uint32_t sh_bits_set(uint32_t data, uint8_t start_bit, size_t bits, uint32_t set)
{
    for (int i = 0; i < bits; i++) {
        data = sh_bits_set_bit(data, start_bit + i, (set >> i) & 1);
    }

    return data;
}











