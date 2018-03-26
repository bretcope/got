#include "GotString.h"

GotString::GotString(char* data, size_t byteCount):
    _data(data),
    _byteCount(byteCount)
{
}

GotString::~GotString()
{
    delete _data;
}

const char* GotString::Value() const
{
    return _data;
}

size_t GotString::ByteLength() const
{
    return _byteCount;
}
