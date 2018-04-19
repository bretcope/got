#include <cassert>
#include <cstring>
#include <algorithm>
#include <memory>
#include "MotString.h"

namespace mot
{
    MotString::Data::Data(SP<const char> sharedStr, const char* str, uint32_t byteLength):
            m_sharedStr{sharedStr},
            m_str{str},
            m_byteLength{byteLength}
    {
    }

    MotString::MotString(mot::SP<mot::MotString::Data> data):
            m_data{data}
    {
    }

    MotString::MotString(SP<const char> sharedStr, uint32_t offset, uint32_t byteLength) :
            m_data{std::make_shared<Data>(sharedStr, &sharedStr.get()[offset], byteLength)}
    {
    }

    MotString::MotString(const char* literal):
            m_data{std::make_shared<Data>(nullptr, literal, (uint32_t)strlen(literal))}
    {
    }

    MotString::MotString() = default;

    std::ostream& operator<<(std::ostream& os, const MotString& s)
    {
        auto len = s.ByteLength();
        if (len > 0)
        {
            os.write(s.m_data->m_str, len);
        }

        return os;
    }

    uint32_t MotString::ByteLength() const
    {
        return m_data == nullptr ? 0 : m_data->m_byteLength;
    }

    uint32_t MotString::CharacterCount() const
    {
        if (m_data == nullptr)
            return 0;

        auto count = m_data->m_charCount;
        if (count == 0 && m_data->m_byteLength > 0 && m_data->m_hashCode == 0)
        {
            CalculateHashAndCharacterCount();
            count = m_data->m_charCount;
        }

        return count;
    }

    uint32_t MotString::HashCode() const
    {
        if (m_data == nullptr)
            return 0;

        auto hash = m_data->m_hashCode;
        if (hash == 0)
        {
            CalculateHashAndCharacterCount();
            hash = m_data->m_hashCode;
        }

        return hash;
    }

    Utf8::Iterator MotString::Iterator() const
    {
        return Utf8::Iterator(m_data == nullptr ? nullptr : m_data->m_str, ByteLength());
    }

    MotString MotString::SubString(uint32_t start, uint32_t length) const
    {
        auto origLen = ByteLength();

        // todo: maybe these should throw exceptions for real
        assert(start <= origLen);
        assert(length <= origLen - start);

        length = start < origLen ? std::min(length, origLen - start) : 0;

        if (length == 0)
            return MotString();

        return MotString(std::make_shared<Data>(m_data->m_sharedStr, &m_data->m_str[start], length));
    }

    int MotString::Compare(const MotString& a, const MotString& b)
    {
        return CompareImpl(a, b, true, true);
    }

    int MotString::CompareCaseInsensitive(const MotString& a, const MotString& b)
    {
        return CompareImpl(a, b, false, true);
    }

    bool MotString::AreEqual(const MotString& a, const MotString& b)
    {
        return CompareImpl(a, b, true, false) == 0;
    }

    bool MotString::AreCaseInsensitiveEqual(const MotString& a, const MotString& b)
    {
        return CompareImpl(a, b, false, false) == 0;
    }

    bool MotString::IsEqualTo(const MotString& str) const
    {
        return AreEqual(*this, str);
    }

    bool MotString::IsCaseInsensitiveEqualTo(const MotString& str) const
    {
        return AreCaseInsensitiveEqual(*this, str);
    }

    bool MotString::IsEmpty(const MotString& s)
    {
        return &s == nullptr || s.ByteLength() == 0;
    }

    int MotString::CompareImpl(const MotString& a, const MotString& b, bool caseSensitive, bool orderedResult)
    {
        if (&a == &b)
            return 0;

        if (&a == nullptr || &b == nullptr)
            return &a == nullptr ? 1 : -1; // we're going to say that any value comes before null

        if (a.m_data == b.m_data)
            return 0;

        auto aLen = a.ByteLength();
        auto bLen = b.ByteLength();

        if (aLen == 0 || bLen == 0)
        {
            if (aLen == bLen)
                return 0;

            return aLen == 0 ? -1 : 1;
        }

        if (!orderedResult && aLen != bLen)
            return 1;

        auto aStr = a.m_data->m_str;
        auto bStr = b.m_data->m_str;

        if (aStr == bStr && aLen == bLen)
            return 0;

        if (!orderedResult && a.HashCode() != b.HashCode())
            return 1;

        auto commonLen = std::min(aLen, bLen);

        if (caseSensitive)
        {
            auto compare = memcmp(aStr, bStr, commonLen);
            if (compare != 0)
                return compare;
        }
        else // case-insensitive
        {
            // this could use some performance work, but another day
            for (auto i = 0u; i < commonLen;)
            {
                char32_t aCh, bCh;
                auto aChLen = Utf8::Decode(aStr, i, aLen, aCh);
                auto bChLen = Utf8::Decode(bStr, i, bLen, bCh);

                if (aChLen == 0 || bChLen == 0)
                {
                    // invalid UTF-8 sequence - we'll fallback to direct byte comparison
                    if (aStr[i] != bStr[i])
                        return (uint8_t)aStr[i] < (uint8_t)bStr[i] ? -1 : 1;

                    i++;
                    continue;
                }

                if (aChLen != bChLen)
                    return aChLen < bChLen ? -1 : 1;

                if (aCh != bCh)
                {
                    aCh = Utf8::ToUpper(aCh);
                    bCh = Utf8::ToUpper(bCh);

                    if (aCh != bCh)
                        return aCh < bCh ? -1 : 1;
                }

                i += aChLen;
            }
        }

        // at this point we're at the end of the common length
        if (aLen == bLen)
            return 0;

        return aLen < bLen ? -1 : 1;
    }

    void MotString::CalculateHashAndCharacterCount() const
    {
        const uint32_t FNV_PRIME = 16777619;
        const uint32_t FNV_OFFSET = 2166136261;

        auto hash = FNV_OFFSET;
        auto length = ByteLength();
        auto charCount = length;
        auto str = m_data->m_str;
        for (auto i = 0u; i < length;)
        {
            auto byte = (unsigned char)str[i];

            if (byte < 0x80u) // fast path for ASCII
            {
                byte = (unsigned char)Utf8::ToUpper(byte);
                hash = (hash | byte) * FNV_PRIME;
                i++;
                continue;
            }

            // slow path - we need to decode the unicode
            char32_t ch;
            auto charLen = Utf8::Decode(str, i, length, ch);
            assert(charLen != 1); // should have been handled by the single byte case above
            assert(charLen <= 4);

            ch = Utf8::ToUpper(ch);
            auto chBytePtr = (unsigned char*)&ch;

            if (charLen == 0)
            {
                // invalid UTF-8 sequence. We'll just treat it as a single byte for the purpose of hashing
                hash = (hash | byte) * FNV_PRIME;
                i++;
                continue;
            }

            if (ch < 0x10000u)
            {
#if MOT_BIG_ENDIAN
                hash = (hash | chBytePtr[2]) * FNV_PRIME;
                hash = (hash | chBytePtr[3]) * FNV_PRIME;
#else
                hash = (hash | chBytePtr[1]) * FNV_PRIME;
                hash = (hash | chBytePtr[0]) * FNV_PRIME;
#endif
            }
            else if (ch < 0x1000000u)
            {
#if MOT_BIG_ENDIAN
                hash = (hash | chBytePtr[1]) * FNV_PRIME;
                hash = (hash | chBytePtr[2]) * FNV_PRIME;
                hash = (hash | chBytePtr[3]) * FNV_PRIME;
#else
                hash = (hash | chBytePtr[2]) * FNV_PRIME;
                hash = (hash | chBytePtr[1]) * FNV_PRIME;
                hash = (hash | chBytePtr[0]) * FNV_PRIME;
#endif
            }
            else
            {
#if MOT_BIG_ENDIAN
                hash = (hash | chBytePtr[0]) * FNV_PRIME;
                hash = (hash | chBytePtr[1]) * FNV_PRIME;
                hash = (hash | chBytePtr[2]) * FNV_PRIME;
                hash = (hash | chBytePtr[3]) * FNV_PRIME;
#else
                hash = (hash | chBytePtr[3]) * FNV_PRIME;
                hash = (hash | chBytePtr[2]) * FNV_PRIME;
                hash = (hash | chBytePtr[1]) * FNV_PRIME;
                hash = (hash | chBytePtr[0]) * FNV_PRIME;
#endif
            }

            i += charLen;
            charCount -= charLen - 1;
        }

        assert(charCount <= length);

        m_data->m_hashCode = hash;
        m_data->m_charCount = charCount;
    }
}
