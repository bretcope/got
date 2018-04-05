#include <cassert>
#include <cstring>
#include <algorithm>
#include "MotString.h"
#include "Utf8.h"

MotString::MotString(const char* data, uint32_t byteCount, bool transferOwnership):
    _data(data),
    _byteCount(byteCount),
    _isOwnerOfData(transferOwnership)
{
    assert(byteCount == 0 || data != nullptr);
}

MotString::~MotString()
{
    if (_isOwnerOfData)
        delete (char*)_data;
}

MotString* MotString::NewFromConstant(const char* str)
{
    auto len = (uint32_t)strlen(str);
    return new MotString(str, len, false);
}

uint32_t MotString::ByteLength() const
{
    return _byteCount;
}

uint32_t MotString::CharacterCount() const
{
    auto count = _charCount;
    if (count == 0 && _byteCount > 0 && _hashCode == 0)
    {
        CalculateHashAndCharacterCount();
        count = _charCount;
    }

    return count;
}

uint32_t MotString::HashCode() const
{
    auto hash = _hashCode;
    if (hash == 0)
    {
        CalculateHashAndCharacterCount();
        hash = _hashCode;
    }

    return hash;
}

int MotString::Compare(const MotString* a, const MotString* b)
{
    return CompareImpl(a, b, true, true);
}

int MotString::CompareCaseInsensitive(const MotString* a, const MotString* b)
{
    return CompareImpl(a, b, false, true);
}

bool MotString::AreEqual(const MotString* a, const MotString* b)
{
    return CompareImpl(a, b, true, false) == 0;
}

bool MotString::AreCaseInsensitiveEqual(const MotString* a, const MotString* b)
{
    return CompareImpl(a, b, false, false) == 0;
}

bool MotString::IsEqualTo(const MotString* str) const
{
    return AreEqual(this, str);
}

bool MotString::IsCaseInsensitiveEqualTo(const MotString* str) const
{
    return AreCaseInsensitiveEqual(this, str);
}

void MotString::Print(FILE* stream) const
{
    fwrite(_data, 1, _byteCount, stream);
}

int MotString::CompareImpl(const MotString* a, const MotString* b, bool caseSensitive, bool orderedResult)
{
    if (a == b)
        return 0;

    auto aLen = a->_byteCount;
    auto bLen = b->_byteCount;

    if (aLen == 0 || bLen == 0)
    {
        if (aLen == bLen)
            return 0;

        return aLen == 0 ? -1 : 1;
    }

    if (!orderedResult && aLen != bLen)
        return 1;

    auto aData = a->_data;
    auto bData = b->_data;

    if (aData == bData)
        return 0;

    if (!orderedResult && a->HashCode() != b->HashCode())
        return 1;

    auto commonLen = std::min(aLen, bLen);

    if (caseSensitive)
    {
        auto compare = memcmp(aData, bData, commonLen);
        if (compare != 0)
            return compare;
    }
    else // case-insensitive
    {
        // todo: this could use some performance work, but another day
        for (auto i = 0u; i < commonLen;)
        {
            char32_t aCh, bCh;
            auto aChLen = Utf8::Decode(aData, i, aLen, aCh);
            auto bChLen = Utf8::Decode(bData, i, bLen, bCh);

            if (aChLen == 0 || bChLen == 0)
            {
                // invalid UTF-8 sequence - we'll fallback to direct byte comparison
                if (aData[i] != bData[i])
                    return (uint8_t)aData[i] < (uint8_t)bData[i] ? -1 : 1;

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
    auto length = _byteCount;
    auto charCount = length;
    auto data = _data;
    for (auto i = 0u; i < length;)
    {
        auto byte = (unsigned char)data[i];

        if (byte < 0x80u) // fast path for ASCII
        {
            byte = (unsigned char)Utf8::ToUpper(byte);
            hash = (hash | byte) * FNV_PRIME;
            i++;
            continue;
        }

        // slow path - we need to decode the unicode
        char32_t ch;
        auto charLen = Utf8::Decode(data, i, length, ch);
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

    _hashCode = hash;
    _charCount = charCount;
}
