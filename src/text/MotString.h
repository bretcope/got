#ifndef MOT_MOTSTRING_H
#define MOT_MOTSTRING_H


#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <ostream>
#include "Utf8.h"
#include "../Common.h"

namespace mot
{
    class MotString
    {
    private:
        class Data
        {
        public:
            SP<const char> m_sharedStr;
            const char* m_str;
            uint32_t m_byteLength;
            uint32_t m_hashCode = 0; // lazy loaded
            uint32_t m_charCount = 0; // lazy loaded

            Data(SP<const char> sharedStr, const char* str, uint32_t byteLength);
        };

        SP<Data> m_data;

        explicit MotString(SP<Data> data);

    public:
        /**
         *
         * @param sharedStr The raw UTF-8 string.
         * @param offset The offset into the shared string where the MotString should start.
         * @param byteLength The number of bytes which are meaningful (excludes the null terminator).
         */
        MotString(SP<const char> sharedStr, uint32_t offset, uint32_t byteLength);

        /**
         * Creates a MotString based off of a null-terminated C string.
         *
         * \warning The MotString does not control the lifetime of the C string. This constructor is only intended for creating MotStrings based off of
         * string literals.
         */
        explicit MotString(const char* literal);

        /**
         * Creates an empty string.
         */
        MotString();

        friend std::ostream& operator<<(std::ostream& os, const MotString& s);

        /**
         * Returns the number of bytes which are meaningful (excludes the null terminator).
         */
        uint32_t ByteLength() const;

        /**
         * Returns the number of unicode code points in the string.
         */
        uint32_t CharacterCount() const;

        /**
         * Returns a case-insensitive hash code for the string.
         */
        uint32_t HashCode() const;

        Utf8::Iterator Iterator() const;

        /**
         * Creates a substring based on a byte offset and length. Does not enforce that the substring starts or ends on valid UTF-8 boundaries.
         * @param start Byte offset where the substring should start.
         * @param length Length (in bytes) of the substring.
         */
        MotString SubString(uint32_t start, uint32_t length) const;

        /**
         * Returns zero if the two strings have identical values, a negative value if a < b, and a positive if a > b. The ordering is based on the binary encoding.
         */
        static int Compare(const MotString& a, const MotString& b);

        /**
         * Compares a and b in a case insensitive way. Returns 0 when a == b, negative when a < b, and positive when a > b. The comparison is based on the
         * uppercase variant of each code point in the string.
         */
        static int CompareCaseInsensitive(const MotString& a, const MotString& b);

        /**
         * Returns true if the two strings have identical values.
         */
        static bool AreEqual(const MotString& a, const MotString& b);

        /**
         * Returns true if the two strings are equal when compared in a case-insensitive way.
         */
        static bool AreCaseInsensitiveEqual(const MotString& a, const MotString& b);

        bool IsEqualTo(const MotString& str) const;

        bool IsCaseInsensitiveEqualTo(const MotString& str) const;

        static bool IsEmpty(const MotString& s);

    private:

        /**
         * Returns zero if the two strings are equivalent. Otherwise, non-zero.
         * @param a String to compare.
         * @param b String to compare.
         * @param caseSensitive If true, the strings must be an exact match.
         * @param orderedResult If true, the return value guaranteed to be negative if a < b and positive if a > b. If false, the return value is only guaranteed
         * to be non-zero when the strings are not equal.
         */
        static int CompareImpl(const MotString& a, const MotString& b, bool caseSensitive, bool orderedResult);

        void CalculateHashAndCharacterCount() const;
    };
}

#endif //MOT_MOTSTRING_H
