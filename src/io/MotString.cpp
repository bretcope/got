#include "MotString.h"

MotString::MotString(char* data, size_t byteCount):
    _data(data),
    _byteCount(byteCount)
{
}

MotString::~MotString()
{
    delete _data;
}

const char* MotString::Value() const
{
    return _data;
}

size_t MotString::ByteLength() const
{
    return _byteCount;
}
