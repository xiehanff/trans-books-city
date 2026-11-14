#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    const uint32_t value = UINT32_C(0x01020304);
    unsigned char bytes[sizeof(value)];
    memcpy(bytes, &value, sizeof(bytes));

    if (CHAR_BIT != 8 || sizeof(value) != 4) {
        printf("byte_order=unsupported sizeof_int=%zu sizeof_pointer=%zu\n",
               sizeof(int), sizeof(void *));
        return 0;
    }

    const char *order;
    if (bytes[0] == 0x04 && bytes[1] == 0x03 &&
        bytes[2] == 0x02 && bytes[3] == 0x01) {
        order = "little";
    } else if (bytes[0] == 0x01 && bytes[1] == 0x02 &&
               bytes[2] == 0x03 && bytes[3] == 0x04) {
        order = "big";
    } else {
        return 2;
    }

    if (sizeof(char) != 1 || sizeof(int) < 2 || sizeof(void *) == 0) {
        return 3;
    }

    printf("byte_order=%s sizeof_int=%zu sizeof_pointer=%zu\n",
           order, sizeof(int), sizeof(void *));
    return 0;
}
