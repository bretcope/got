#include <cassert>
#include <cstring>
#include "FileSpan.h"

namespace mot
{
    FileSpan::FileSpan(const FileContent* content, uint32_t start, uint32_t end) :
            _content(content),
            _start(start),
            _end(end)
    {
        assert(content != nullptr);
        assert(end <= content->Size());
        assert(end >= start);
    }

    FileSpan::~FileSpan() = default;

    std::ostream& operator<<(std::ostream& os, FileSpan& span)
    {
        auto length = span.Length();
        if (length > 0)
            os.write(&span._content->Data()[span._start], length);

        return os;
    }

    const FileContent* FileSpan::Content() const
    {
        return _content;
    }

    uint32_t FileSpan::Start() const
    {
        return _start;
    }

    uint32_t FileSpan::End() const
    {
        return _end;
    }

    uint32_t FileSpan::Length() const
    {
        return _end - _start;
    }

    uint32_t FileSpan::CopyTo(char* dest) const
    {
        auto length = Length();

        if (length == 0)
            return 0;

        memcpy(dest, &_content->Data()[_start], Length());
        return length;
    }

    MotString* FileSpan::NewMotString() const
    {
        auto length = Length();
        char* str;

        if (length > 0)
        {
            str = new char[length];
            memcpy(str, &_content->Data()[_start], length);
        }
        else
        {
            str = nullptr;
        }

        return new MotString(str, length, true);
    }
}
