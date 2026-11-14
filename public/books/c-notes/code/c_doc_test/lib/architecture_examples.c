#include "architecture_examples.h"

#include <stddef.h>
#include <string.h>

ArchitectureByteOrder architecture_detect_byte_order(void)
{
    const uint32_t value = UINT32_C(0x01020304);
    unsigned char bytes[sizeof value];

    memcpy(bytes, &value, sizeof bytes);
    if (bytes[0] == 0x04U) {
        return ARCHITECTURE_LITTLE_ENDIAN;
    }
    if (bytes[0] == 0x01U) {
        return ARCHITECTURE_BIG_ENDIAN;
    }
    return ARCHITECTURE_OTHER_ENDIAN;
}

void architecture_copy_bytes(uint32_t value, unsigned char *bytes,
                             size_t capacity)
{
    if (bytes == NULL || capacity < sizeof value) {
        return;
    }
    memcpy(bytes, &value, sizeof value);
}

int architecture_pointer_round_trip(int value)
{
    int local = value;
    int *pointer = &local;

    return pointer == &local && *pointer == value;
}

void architecture_write_through_pointer(int *value, int replacement)
{
    if (value != NULL) {
        *value = replacement;
    }
}

size_t architecture_record_size(void)
{
    return sizeof(ArchitectureRecord);
}

size_t architecture_record_value_offset(void)
{
    return offsetof(ArchitectureRecord, value);
}
