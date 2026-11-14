#include "architecture_examples.h"
#include "test_common.h"

#include <string.h>

void test_architecture_examples(void)
{
    const uint32_t value = UINT32_C(0x01020304);
    unsigned char bytes[sizeof value] = {0};
    ArchitectureByteOrder expected_order;
    int value_for_pointer = 10;
    int *pointer = &value_for_pointer;

    architecture_copy_bytes(value, bytes, sizeof bytes);
    {
        uint32_t rebuilt = 0U;
        memcpy(&rebuilt, bytes, sizeof rebuilt);
        TEST_ASSERT(rebuilt == value);
    }

    if (bytes[0] == 0x04U) {
        expected_order = ARCHITECTURE_LITTLE_ENDIAN;
    } else if (bytes[0] == 0x01U) {
        expected_order = ARCHITECTURE_BIG_ENDIAN;
    } else {
        expected_order = ARCHITECTURE_OTHER_ENDIAN;
    }
    TEST_ASSERT(architecture_detect_byte_order() == expected_order);

    TEST_ASSERT(architecture_pointer_round_trip(123) != 0);
    TEST_ASSERT((void *)pointer == (void *)&value_for_pointer);
    architecture_write_through_pointer(pointer, 25);
    TEST_ASSERT(value_for_pointer == 25);

    TEST_ASSERT(architecture_record_value_offset() + sizeof(int) <=
                architecture_record_size());
}
