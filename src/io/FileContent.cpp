#include <algorithm>
#include <cstring>
#include <cassert>
#include "FileContent.h"
#include "../text/Utf8.h"

namespace mot
{
    FileContent::FileContent(char* filename, char* data, uint32_t size) :
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

    void FileContent::PositionDetails(uint32_t position, uint32_t* out_lineNumber, uint32_t* out_lineStart, uint32_t* out_column) const
    {
        assert(out_lineNumber != nullptr || out_lineStart != nullptr || out_column != nullptr); // doesn't make sense to call this method if everything is null

        auto count = _lineCount;
        if (count == 0)
        {
            if (out_lineNumber != nullptr)
                *out_lineNumber = 0;

            if (out_lineStart != nullptr)
                *out_lineStart = 0;

            if (out_column != nullptr)
                *out_column = 0;

            return;
        }

        auto lines = _lineStarts;
        assert(lines[0] == 0);
        auto lineStart = 0u;

        // perform binary search to find the line number
        auto left = 0u;
        auto right = count;
        auto i = right / 2;
        while (true)
        {
            assert(i < right);
            assert(i >= left);
            assert(right <= count);

            lineStart = lines[i];

            if (position >= lineStart && (i + 1 == count || position < lines[i + 1]))
            {
                if (out_lineNumber != nullptr)
                    *out_lineNumber = i;

                if (out_lineStart != nullptr)
                    *out_lineStart = lineStart;

                break;
            }

            if (position < lineStart)
            {
                // try earlier in the list
                assert(i != 0);
                right = i;
            }
            else
            {
                // try later in the list
                assert(i + 1 < right);
                left = i + 1;
            }

            i = left + (right - left) / 2;
        }

        assert(position >= lineStart);
        assert(_size >= lineStart);

        if (out_column != nullptr)
        {
            // find character offset within line
            auto length = std::min(position, _size) - lineStart;
            *out_column = length == 0 ? 0 : Utf8::CountCharacters(&_data[lineStart], length);
        }
    }

    uint32_t FileContent::LineStartPosition(uint32_t lineNumber) const
    {
        if (lineNumber < _lineCount)
            return _lineStarts[lineNumber];

        return _size;
    }

    void FileContent::ResetLineMarkers()
    {
        delete[] _lineStarts;
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
}
