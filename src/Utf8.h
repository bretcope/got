#ifndef MOT_UTF8_H
#define MOT_UTF8_H

#include <cstdint>

namespace Utf8
{
    /**
     * Returns the expected size of a UTF-8 encoded character, based on its first byte. Returns zero if the first byte is invalid.
     */
    uint32_t ExpectedSize(char firstByte);

    /**
     * Returns the size (in bytes) of the character when UTF-8 encoded. Returns zero when the character is outside the unicode range.
     */
    uint32_t EncodedSize(char32_t ch);

    /**
     * UTF-8 encodes the character and writes it to the buffer. The buffer must have at least as much room as the value returned from EncodedSize() for the
     * same character.
     * @param ch Character to encode.
     * @param buffer Destination of UTF-8 encoded character.
     * @return The number of bytes written. Returns zero (and writes nothing) if the character is outside the unicode range.
     */
    uint32_t Encode(char32_t ch, char* buffer);
}


#endif //MOT_UTF8_H
