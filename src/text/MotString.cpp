#include <cassert>
#include <cstring>
#include <algorithm>
#include "MotString.h"

namespace mot
{
    MotString::MotString(const char* data, uint32_t byteCount, bool transferOwnership) :
            _data(data),
            _byteCount(byteCount),
            _isOwnerOfData(transferOwnership)
    {
        assert(byteCount == 0 || data != nullptr);
    }

    MotString::MotString(const char* str)
    {
        _data = str;
        _byteCount = (uint32_t)strlen(str);
        _isOwnerOfData = false;
    }

    MotString::MotString():
        _data(nullptr),
        _byteCount(0),
        _isOwnerOfData(false)
    {
    }

    MotString::MotString(const MotString& other)
    {
        auto data = other._data;
        auto len = other._byteCount;
        auto owner = other._isOwnerOfData;

        if (owner && len > 0)
        {
            // need to make a copy of the data
            auto newData = new char[len];
            memcpy(newData, data, len);
            data = newData;
        }

        _data = data;
        _byteCount = len;
        _hashCode = other._hashCode;
        _charCount = other._charCount;
        _isOwnerOfData = owner;
    }

    MotString::MotString(MotString&& other) noexcept
    {
        _data = other._data;
        _byteCount = other._byteCount;
        _hashCode = other._hashCode;
        _charCount = other._charCount;
        _isOwnerOfData = other._isOwnerOfData;

        other._data = nullptr;
    }

    MotString::~MotString()
    {
        if (_isOwnerOfData)
            delete (char*)_data;
    }

    MotString& MotString::operator=(MotString rhs) noexcept
    {
        swap(*this, rhs);
        return *this;
    }

    std::ostream& operator<<(std::ostream& os, const MotString& s)
    {
        auto len = s._byteCount;
        if (len > 0)
        {
            os.write(s._data, len);
        }

        return os;
    }

    std::ostream& operator<<(std::ostream& os, const MotString* s)
    {
        return os << *s;
    }

    void swap(MotString& a, MotString& b) noexcept
    {
        using std::swap;
        swap(a._data, b._data);
        swap(a._byteCount, b._byteCount);
        swap(a._hashCode, b._hashCode);
        swap(a._charCount, b._charCount);
        swap(a._isOwnerOfData, b._isOwnerOfData);
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

    Utf8::Iterator MotString::Iterator() const
    {
        return Utf8::Iterator(_data, _byteCount);
    }

    void MotString::SubString(uint32_t start, uint32_t length, bool copyData, mot::MotString& out_subString) const
    {
        auto origLen = ByteLength();

        assert(start <= origLen);
        assert(length <= origLen - start);

        length = start < origLen ? std::min(length, origLen - start) : 0;

        if (length == 0)
        {
            out_subString = MotString();
            return;
        }

        const char* data;
        if (copyData)
        {
            data = new char[length];
            memcpy((char*)data, &_data[start], length);
        }
        else
        {
            data = &_data[start];
        }

        out_subString = MotString(data, length, copyData);
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

    const MotString* MotString::Empty()
    {
        static const MotString empty;
        return &empty;
    }

    bool MotString::IsEmpty(const MotString* s)
    {
        return s == nullptr || s->ByteLength() == 0;
    }

    int MotString::CompareImpl(const MotString* a, const MotString* b, bool caseSensitive, bool orderedResult)
    {
        if (a == b)
            return 0;

        if (a == nullptr || b == nullptr)
        {
            return a == nullptr ? 1 : -1; // we're going to say that any value comes before null
        }

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
            // this could use some performance work, but another day
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
}
