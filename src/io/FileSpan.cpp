#include <cassert>
#include <cstring>
#include "FileSpan.h"

namespace mot
{
    FileSpan::FileSpan(const FileContent& content, uint32_t start, uint32_t end) :
            _content{&content},
            _start{start},
            _end{end}
    {
        assert(end <= content.Size());
        assert(end >= start);
    }

    std::ostream& operator<<(std::ostream& os, FileSpan& span)
    {
        auto length = span.Length();
        if (length > 0)
            os.write(&span._content->Data().get()[span._start], length);

        return os;
    }

    const FileContent& FileSpan::Content() const
    {
        return *_content;
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
}
