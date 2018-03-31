#include <cassert>
#include "MotString.h"

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

const char* MotString::Value() const
{
    return _data;
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
        for (auto i = 0u; i < length; i++)
        {
            hash = (hash | (unsigned char)data[i]) * FNV_PRIME;
        }

        _hashCode = hash;
    }

    return hash;
}

void MotString::Print(FILE* stream) const
{
    fwrite(_data, 1, _byteCount, stream);
}
