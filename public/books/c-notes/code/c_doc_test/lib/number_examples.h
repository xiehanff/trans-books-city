#ifndef C_DOC_NUMBER_EXAMPLES_H
#define C_DOC_NUMBER_EXAMPLES_H

#include <stdint.h>

uint32_t number_octal_value(void);
uint32_t number_hexadecimal_value(void);
uint32_t number_bitwise_and(uint32_t left, uint32_t right);
uint32_t number_bitwise_or(uint32_t left, uint32_t right);
uint32_t number_bitwise_xor(uint32_t left, uint32_t right);
uint32_t number_set_bit(uint32_t value, unsigned int bit);
uint32_t number_clear_bit(uint32_t value, unsigned int bit);
unsigned int number_get_bit(uint32_t value, unsigned int bit);
uint32_t number_minus_one_pattern(void);
uint8_t number_increment_byte(uint8_t value);

#endif
