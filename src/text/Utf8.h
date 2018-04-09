#ifndef MOT_UTF8_H
#define MOT_UTF8_H

#include <cstdint>

namespace mot
{
    namespace Utf8
    {
        /**
         * Returns the expected size of a UTF-8 encoded character, based on its first byte. Returns zero if the first byte is invalid.
         */
        int ExpectedSize(char firstByte);

        /**
         * Returns the size (in bytes) of the character when UTF-8 encoded. Returns zero when the character is outside the unicode range.
         */
        int EncodedSize(char32_t ch);

        /**
         * UTF-8 encodes the character and writes it to the buffer. The buffer must have at least as much room as the value returned from EncodedSize() for the
         * same character.
         * @param ch Character to encode.
         * @param buffer Destination of UTF-8 encoded character.
         * @return The number of bytes written. Returns zero (and writes nothing) if the character is outside the unicode range.
         */
        int Encode(char32_t ch, char* buffer);

        /**
         * Decodes a UTF-8 code point, and returns the number of bytes consumed. Zero is returned for invalid code points.
         */
        int Decode(const char* str, uint32_t position, uint32_t size, char32_t& out_char);

        /**
         * Returns the number of unicode characters in the given sequence of bytes. Byte sequences which are not valid UTF-8 are counted per byte.
         */
        uint32_t CountCharacters(const char* data, uint32_t byteLength);

        /**
         * Returns the uppercase variant of the code point. If no conversion is applicable, the original character is returned.
         */
        char32_t ToUpper(char32_t ch);

        /**
         * Returns the lowercase variant of the code point. If no conversion is applicable, the original character is returned.
         */
        char32_t ToLower(char32_t ch);

        class Iterator
        {
        private:
            const char* _data;
            uint32_t _index;
            uint32_t _length;
            char32_t _currentChar;
            uint32_t _chSize;
            bool _isValidUtf8;

        public:
            Iterator(const char* data, uint32_t length);

            Iterator& operator++();
            char32_t operator*() const;
            bool HasMore() const;
            bool IsValidUtf8() const;
            bool ShouldContinue() const;
            uint32_t Index() const;

        private:
            void SetCurrentChar();
        };
    }
}

#endif //MOT_UTF8_H
