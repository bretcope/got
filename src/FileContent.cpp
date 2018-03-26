#include <algorithm>
#include <cstring>
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
