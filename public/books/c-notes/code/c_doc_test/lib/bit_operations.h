#ifndef C_DOC_TEST_BIT_OPERATIONS_H
#define C_DOC_TEST_BIT_OPERATIONS_H

#include <stdint.h>

#define PERM_READ UINT32_C(1)
#define PERM_WRITE UINT32_C(2)
#define PERM_EXECUTE UINT32_C(4)
#define PERM_DELETE UINT32_C(8)

int get_bit(uint32_t value, unsigned int bit);
uint32_t set_bit(uint32_t value, unsigned int bit);
uint32_t clear_bit(uint32_t value, unsigned int bit);
uint32_t toggle_bit(uint32_t value, unsigned int bit);

uint8_t invert_byte(uint8_t value);
uint32_t mul2(uint32_t value);
uint32_t div2(uint32_t value);
uint32_t mod_power2(uint32_t value, uint32_t divisor);
int is_power2(uint32_t value);

uint8_t color_red(uint32_t color);
uint8_t color_green(uint32_t color);
uint8_t color_blue(uint32_t color);
uint32_t color_rgb(uint8_t red, uint8_t green, uint8_t blue);

#endif
