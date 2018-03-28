#include <algorithm>
#include <cstring>
#include <cassert>
#include "FileContent.h"

FileContent::FileContent(char* filename, char* data, uint32_t size):
        _filename(filename),
        _data(data),
        _size(size)
{
}

FileContent::~FileContent()
{
    delete _filename;
    delete _data;
    delete _lineStarts;
}

const char* FileContent::Filename() const
{
    return _filename;
}

const char* FileContent::Data() const
{
    return _data;
}

uint32_t FileContent::Size() const
{
    return _size;
}

uint32_t FileContent::LineCount() const
{
    return _lineCount;
}

uint32_t FileContent::LineNumber(uint32_t position, uint32_t& out_lineStart) const
{
    auto count = _lineCount;
    if (count == 0)
    {
        out_lineStart = 0;
        return 0;
    }

    auto lines = _lineStarts;
    assert(lines[0] == 0);

    // perform binary search to find the line number
    auto i = count / 2;
    while (true)
    {
        assert(i < count);
        auto start = lines[i];

        if (position >= start && (i + 1 == count || position < lines[i + 1]))
        {
            out_lineStart = start;
            return i;
        }

        if (position < start)
        {
            // try earlier in the list
            assert(i != 0);
            i = i / 2;
        }
        else
        {
            // try later in the list
            assert(i + 1 < count);
            i += (count - i) / 2;
        }
    }
}

void FileContent::ResetLineMarkers()
{
    delete _lineStarts;
    _lineStarts = nullptr;
    _lineCount = 0;
}

uint32_t FileContent::MarkLine(uint32_t position)
{
    auto lineNumber = _lineCount;
    auto lineStarts = _lineStarts;

    assert(position == 0 || lineNumber > 0); // first line must start at position 0

    if (lineNumber >= _lineCapacity)
    {
        // we need more room to store the next line start
        auto newCapacity = std::max(60u, (lineNumber + 1) * 2);
        auto newLineStarts = new uint32_t[newCapacity];

        if (lineStarts != nullptr)
        {
            // copy existing line start data
            memcpy(newLineStarts, lineStarts, lineNumber * sizeof(uint32_t));
            delete lineStarts;
        }

        _lineStarts = newLineStarts;
        lineStarts = newLineStarts;
    }

    lineStarts[lineNumber] = position;
    _lineCount = lineNumber + 1;

    return lineNumber;
}
