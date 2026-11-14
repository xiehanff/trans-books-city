#include "number_examples.h"
#include "test_common.h"

void test_number_examples(void)
{
    const uint32_t left = UINT32_C(0xCA);
    const uint32_t right = UINT32_C(0xAC);
    const uint32_t bit_value = UINT32_C(0x10);

    TEST_ASSERT(number_octal_value() == UINT32_C(493));
    TEST_ASSERT(number_hexadecimal_value() == UINT32_C(57005));
    TEST_ASSERT(number_bitwise_and(left, right) == UINT32_C(0x88));
    TEST_ASSERT(number_bitwise_or(left, right) == UINT32_C(0xEE));
    TEST_ASSERT(number_bitwise_xor(left, right) == UINT32_C(0x66));

    TEST_ASSERT(number_set_bit(0U, 4U) == bit_value);
    TEST_ASSERT(number_clear_bit(UINT32_MAX, 4U) == UINT32_MAX - bit_value);
    TEST_ASSERT(number_get_bit(bit_value, 4U) == 1U);
    TEST_ASSERT(number_get_bit(bit_value, 3U) == 0U);
    TEST_ASSERT(number_set_bit(1U, 32U) == 1U);
    TEST_ASSERT(number_clear_bit(1U, 32U) == 1U);
    TEST_ASSERT(number_get_bit(1U, 32U) == 0U);

    TEST_ASSERT(number_minus_one_pattern() == UINT32_MAX);
    TEST_ASSERT(number_increment_byte(UINT8_MAX) == UINT8_C(0));
    TEST_ASSERT(number_increment_byte(UINT8_C(41)) == UINT8_C(42));
}
