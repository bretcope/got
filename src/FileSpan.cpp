#include "FileSpan.h"

FileSpan::FileSpan():
        _content(nullptr),
        _start(0),
        _end(0)
{
}

FileSpan::FileSpan(const FileContent* content, uint32_t start, uint32_t end):
        _content(content),
        _start(start),
        _end(end)
{
}

FileSpan::~FileSpan() = default;

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
