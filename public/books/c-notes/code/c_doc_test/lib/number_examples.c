#include "number_examples.h"

uint32_t number_octal_value(void)
{
    return UINT32_C(0755);
}

uint32_t number_hexadecimal_value(void)
{
    return UINT32_C(0xDEAD);
}

uint32_t number_bitwise_and(uint32_t left, uint32_t right)
{
    return left & right;
}

uint32_t number_bitwise_or(uint32_t left, uint32_t right)
{
    return left | right;
}

uint32_t number_bitwise_xor(uint32_t left, uint32_t right)
{
    return left ^ right;
}

uint32_t number_set_bit(uint32_t value, unsigned int bit)
{
    if (bit >= 32U) {
        return value;
    }
    return value | (UINT32_C(1) << bit);
}

uint32_t number_clear_bit(uint32_t value, unsigned int bit)
{
    if (bit >= 32U) {
        return value;
    }
    return value & ~(UINT32_C(1) << bit);
}

unsigned int number_get_bit(uint32_t value, unsigned int bit)
{
    if (bit >= 32U) {
        return 0U;
    }
    return (unsigned int)((value >> bit) & UINT32_C(1));
}

uint32_t number_minus_one_pattern(void)
{
    return (uint32_t)-1;
}

uint8_t number_increment_byte(uint8_t value)
{
    return (uint8_t)(value + UINT8_C(1));
}
