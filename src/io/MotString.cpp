#include <cassert>
#include <cstring>
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

uint32_t MotString::HashCode() const
{
    auto hash = _hashCode;
    if (hash == 0)
    {
        const uint32_t FNV_PRIME = 16777619;
        const uint32_t FNV_OFFSET = 2166136261;

        hash = FNV_OFFSET;
        auto length = _byteCount;
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
        }

        _hashCode = hash;
    }

    return hash;
}

void MotString::Print(FILE* stream) const
{
    fwrite(_data, 1, _byteCount, stream);
}
