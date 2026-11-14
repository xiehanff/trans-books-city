#ifndef C_DOC_ARCHITECTURE_EXAMPLES_H
#define C_DOC_ARCHITECTURE_EXAMPLES_H

#include <stddef.h>
#include <stdint.h>

typedef enum ArchitectureByteOrder {
    ARCHITECTURE_LITTLE_ENDIAN,
    ARCHITECTURE_BIG_ENDIAN,
    ARCHITECTURE_OTHER_ENDIAN
} ArchitectureByteOrder;

typedef struct ArchitectureRecord {
    char tag;
    int value;
} ArchitectureRecord;

ArchitectureByteOrder architecture_detect_byte_order(void);
void architecture_copy_bytes(uint32_t value, unsigned char *bytes,
                             size_t capacity);
int architecture_pointer_round_trip(int value);
void architecture_write_through_pointer(int *value, int replacement);
size_t architecture_record_size(void);
size_t architecture_record_value_offset(void);

#endif
