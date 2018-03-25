#ifndef GOT_UNICODECHAR_H
#define GOT_UNICODECHAR_H

#include <cstdint>

struct Utf8Char
{
    char Data[4];
    uint32_t Size;
};

/**
 * Returns the number of bytes expected for a UTF-8 code point based on its first byte. If
 */
uint32_t GetUtf8Size(char firstByte)
{
    const uint8_t MASK_1 =  0b10000000;
    const uint8_t MASK_2 =  0b11100000;
    const uint8_t MASK_3 =  0b11110000;
    const uint8_t MASK_4 =  0b11111000;

    const uint8_t VALUE_1 = 0b00000000;
    const uint8_t VALUE_2 = 0b11000000;
    const uint8_t VALUE_3 = 0b11100000;
    const uint8_t VALUE_4 = 0b11110000;

    auto uChar = (uint8_t)firstByte;

    if ((uChar & MASK_1) == VALUE_1)
        return 1;

    if ((uChar & MASK_2) == VALUE_2)
        return 2;

    if ((uChar & MASK_3) == VALUE_3)
        return 3;

    if ((uChar & MASK_4) == VALUE_4)
        return 4;

    return 0;
}

#endif //GOT_UNICODECHAR_H
