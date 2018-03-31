#include <algorithm>
#include "Utf8.h"

int Utf8::ExpectedSize(char firstByte)
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

const uint32_t MAX_1_BYTE = 0x7F;
const uint32_t MAX_2_BYTE = 0x7FF;
const uint32_t MAX_3_BYTE = 0xFFFF;
const uint32_t MAX_4_BYTE = 0x10FFFF;

int Utf8::EncodedSize(char32_t ch)
{
    if (ch <= MAX_1_BYTE)
        return 1;

    if (ch <= MAX_2_BYTE)
        return 2;

    if (ch <= MAX_3_BYTE)
        return 3;

    if (ch <= MAX_4_BYTE)
        return 4;

    return 0;
}

int Utf8::Encode(char32_t ch, char* buffer)
{
    if (ch <= MAX_1_BYTE)
    {
        *buffer = (uint8_t)ch;
        return 1;
    }

    if (ch <= MAX_2_BYTE)
    {
        buffer[0] = (uint8_t)(0b11000000u | (ch >> 6u));
        buffer[1] = (uint8_t)(0b10000000u | (ch & 0x3Fu));
        return 2;
    }

    if (ch <= MAX_3_BYTE)
    {
        buffer[0] = (uint8_t)(0b11100000u | (ch >> 12u));
        buffer[1] = (uint8_t)(0b10000000u | ((ch >> 6u) & 0x3Fu));
        buffer[2] = (uint8_t)(0b10000000u | (ch & 0x3Fu));
        return 3;
    }

    if (ch <= MAX_4_BYTE)
    {
        buffer[0] = (uint8_t)(0b11110000u | (ch >> 24u));
        buffer[1] = (uint8_t)(0b10000000u | ((ch >> 12u) & 0x3Fu));
        buffer[2] = (uint8_t)(0b10000000u | ((ch >> 6u) & 0x3Fu));
        buffer[3] = (uint8_t)(0b10000000u | (ch & 0x3Fu));
        return 4;
    }

    return 0;
}

uint32_t Utf8::CountCharacters(const char* data, uint32_t byteLength)
{
    auto i = 0u;

    // quick optimization for ASCII-heavy files
    while (i + 8 <= byteLength)
    {
        if ((*(uint64_t*)&data[i] & 0x8080808080808080ull) == 0ull)
        {
            i += 8;
        }
        else
        {
            break;
        }
    }

    auto count = i;
    while (i < byteLength)
    {
        auto byte = (unsigned char)data[i];
        i += std::max(1, ExpectedSize(byte));
        count++;
    }

    return count;
}
