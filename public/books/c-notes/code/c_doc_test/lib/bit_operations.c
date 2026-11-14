#include "bit_operations.h"

int get_bit(uint32_t value, unsigned int bit) {
    if (bit >= 32U) {
        return 0;
    }
    return (int)((value >> bit) & UINT32_C(1));
}

uint32_t set_bit(uint32_t value, unsigned int bit) {
    if (bit >= 32U) {
        return value;
    }
    return value | (UINT32_C(1) << bit);
}

uint32_t clear_bit(uint32_t value, unsigned int bit) {
    if (bit >= 32U) {
        return value;
    }
    return value & ~(UINT32_C(1) << bit);
}

uint32_t toggle_bit(uint32_t value, unsigned int bit) {
    if (bit >= 32U) {
        return value;
    }
    return value ^ (UINT32_C(1) << bit);
}

uint8_t invert_byte(uint8_t value) {
    return (uint8_t)~value;
}

uint32_t mul2(uint32_t value) {
    return value << 1;
}

uint32_t div2(uint32_t value) {
    return value >> 1;
}

uint32_t mod_power2(uint32_t value, uint32_t divisor) {
    if (!is_power2(divisor)) {
        return 0;
    }
    return value & (divisor - UINT32_C(1));
}

int is_power2(uint32_t value) {
    return value != 0 && (value & (value - UINT32_C(1))) == 0;
}

uint8_t color_red(uint32_t color) {
    return (uint8_t)((color >> 16) & UINT32_C(0xff));
}

uint8_t color_green(uint32_t color) {
    return (uint8_t)((color >> 8) & UINT32_C(0xff));
}

uint8_t color_blue(uint32_t color) {
    return (uint8_t)(color & UINT32_C(0xff));
}

uint32_t color_rgb(uint8_t red, uint8_t green, uint8_t blue) {
    return ((uint32_t)red << 16) | ((uint32_t)green << 8) |
           (uint32_t)blue;
}
