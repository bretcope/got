#include "Utf8.h"

namespace mot
{
    Utf8::Iterator::Iterator(const char* data, uint32_t length):
            _data(data),
            _index(0),
            _length(length),
            _isValidUtf8(true)
    {
        SetCurrentChar();
    }

    Utf8::Iterator& Utf8::Iterator::operator++()
    {
        _index += _chSize;
        SetCurrentChar();
        return *this;
    }

    char32_t Utf8::Iterator::operator*() const
    {
        return _currentChar;
    }

    bool Utf8::Iterator::HasMore() const
    {
        return _index < _length;
    }

    bool Utf8::Iterator::IsValidUtf8() const
    {
        return _isValidUtf8;
    }

    bool Utf8::Iterator::ShouldContinue() const
    {
        return HasMore() && IsValidUtf8();
    }

    uint32_t Utf8::Iterator::Index() const
    {
        return _index;
    }

    void Utf8::Iterator::SetCurrentChar()
    {
        auto index = _index;
        auto length = _length;
        if (index < length)
        {
            auto chSize = Utf8::Decode(_data, index, length, _currentChar);
            if (chSize > 0)
            {
                _chSize = (uint32_t)chSize;
            }
            else
            {
                // invalid code point
                _isValidUtf8 = false;
                _chSize = 1;
            }
        }
        else
        {
            _chSize = 0;
        }
    }
}