#include "test_common.h"

#include "bit_operations.h"

void test_bit_operations(void) {
    /* 第16章：使用无符号整数进行可移植的位掩码操作。 */
    const uint32_t initial = UINT32_C(0xA0);
    TEST_ASSERT(get_bit(initial, 5) == 1);
    TEST_ASSERT(get_bit(initial, 4) == 0);
    TEST_ASSERT(get_bit(initial, 32) == 0);
    TEST_ASSERT(set_bit(initial, 0) == UINT32_C(0xA1));
    TEST_ASSERT(clear_bit(initial, 5) == UINT32_C(0x80));
    TEST_ASSERT(toggle_bit(initial, 4) == UINT32_C(0xB0));
    TEST_ASSERT(set_bit(initial, 32) == initial);
    TEST_ASSERT(clear_bit(initial, 32) == initial);
    TEST_ASSERT(toggle_bit(initial, 32) == initial);

    TEST_ASSERT(invert_byte(UINT8_C(0xAA)) == UINT8_C(0x55));
    TEST_ASSERT(mul2(UINT32_C(5)) == UINT32_C(10));
    TEST_ASSERT(div2(UINT32_C(17)) == UINT32_C(8));
    TEST_ASSERT(mod_power2(UINT32_C(19), UINT32_C(8)) == UINT32_C(3));
    TEST_ASSERT(mod_power2(UINT32_C(19), UINT32_C(0)) == 0);
    TEST_ASSERT(is_power2(1));
    TEST_ASSERT(is_power2(1024));
    TEST_ASSERT(!is_power2(0));
    TEST_ASSERT(!is_power2(12));

    /* 第16章：用各个位组合权限标志，并提取24位RGB颜色分量。 */
    uint32_t permissions = PERM_READ | PERM_WRITE;
    TEST_ASSERT((permissions & PERM_READ) != 0);
    TEST_ASSERT((permissions & PERM_EXECUTE) == 0);
    permissions &= ~PERM_WRITE;
    permissions ^= PERM_EXECUTE;
    TEST_ASSERT(permissions == (PERM_READ | PERM_EXECUTE));

    uint32_t orange = color_rgb(UINT8_C(255), UINT8_C(128), UINT8_C(0));
    TEST_ASSERT(orange == UINT32_C(0xFF8000));
    TEST_ASSERT(color_red(orange) == UINT8_C(255));
    TEST_ASSERT(color_green(orange) == UINT8_C(128));
    TEST_ASSERT(color_blue(orange) == UINT8_C(0));
}
