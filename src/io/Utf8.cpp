#include <algorithm>
#include "Utf8.h"

namespace mot
{
    const uint32_t MAX_1_BYTE = 0x7F;
    const uint32_t MAX_2_BYTE = 0x7FF;
    const uint32_t MAX_3_BYTE = 0xFFFF;
    const uint32_t MAX_4_BYTE = 0x10FFFF;

    const uint8_t MASK_2 = 0b11100000u;
    const uint8_t MASK_3 = 0b11110000u;
    const uint8_t MASK_4 = 0b11111000u;

    const uint8_t FLAGS_2 = 0b11000000u;
    const uint8_t FLAGS_3 = 0b11100000u;
    const uint8_t FLAGS_4 = 0b11110000u;

    int Utf8::ExpectedSize(char firstByte)
    {

        auto uChar = (uint8_t)firstByte;

        if (uChar <= MAX_1_BYTE)
            return 1;

        if ((uChar & MASK_2) == FLAGS_2)
            return 2;

        if ((uChar & MASK_3) == FLAGS_3)
            return 3;

        if ((uChar & MASK_4) == FLAGS_4)
            return 4;

        return 0;
    }

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

    int Utf8::Decode(const char* str, uint32_t position, uint32_t size, char32_t& out_char)
    {
        // This function could be more performant, especially with some endian-specific code, but that's a pet project for another day.

        if (position < size)
        {
            auto firstByte = (unsigned char)str[position];

            if (firstByte <= MAX_1_BYTE)
            {
                out_char = firstByte;
                return 1;
            }

            if ((firstByte & MASK_2) == FLAGS_2)
            {
                if (position + 2 > size)
                    goto INVALID_CHAR;

                auto result = ((char32_t)firstByte & 0b00011111u) << 6u;

                auto temp = (char32_t)str[position + 1];
                result |= temp & 0b00111111u;

                if ((temp & 0b11000000u) != 0b10000000u)
                    goto INVALID_CHAR;

                out_char = result;
                return 2;
            }

            if ((firstByte & MASK_3) == FLAGS_3)
            {
                if (position + 3 > size)
                    goto INVALID_CHAR;

                auto result = ((char32_t)firstByte & 0b00001111u) << 12u;

                auto temp = (char32_t)str[position + 1];
                result |= (temp & 0b00111111u) << 6u;
                temp <<= 8u;

                temp |= (char32_t)str[position + 2];
                result |= (temp & 0b00111111u);

                if ((temp & 0b1100000011000000u) != 0b1000000010000000u)
                    goto INVALID_CHAR;

                out_char = result;
                return 3;
            }

            if ((firstByte & MASK_4) == FLAGS_4)
            {
                if (position + 4 > size)
                    goto INVALID_CHAR;

                auto result = ((char32_t)firstByte & 0b00000111u) << 18u;

                auto temp = (char32_t)str[position + 1];
                result |= (temp & 0b00111111u) << 12u;
                temp <<= 8u;

                temp |= (char32_t)str[position + 2];
                result |= (temp & 0b00111111u) << 6u;
                temp <<= 8u;

                temp |= (char32_t)str[position + 3];
                result |= (temp & 0b00111111u);

                if ((temp & 0b110000001100000011000000u) != 0b100000001000000010000000u)
                    goto INVALID_CHAR;

                out_char = result;
                return 4;
            }
        }

        INVALID_CHAR:
        out_char = 0;
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
}
