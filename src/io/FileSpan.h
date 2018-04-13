#ifndef MOT_SPAN_H
#define MOT_SPAN_H

#include <cstdint>
#include <cstdio>
#include "FileContent.h"
#include "../text/MotString.h"

namespace mot
{
    class FileSpan
    {
    private:
        const FileContent* _content;
        uint32_t _start;
        uint32_t _end;

    public:
        FileSpan(const FileContent* content, uint32_t start, uint32_t end);
        ~FileSpan();

        friend std::ostream& operator<<(std::ostream& os, FileSpan& span);

        const FileContent* Content() const;

        /**
         * Inclusive Start position (byte offset from the document's start).
         */
        uint32_t Start() const;

        /**
         * Exclusive End position (byte offset from the document's start).
         */
        uint32_t End() const;

        /**
         * The length (in bytes) of the span.
         */
        uint32_t Length() const;

        /**
         * Copies the characters from the span to the destination char array (does not include a null terminator).
         * @return The number of bytes copied (equal to Length()).
         */
        uint32_t CopyTo(char* dest) const;

        /**
         * Allocates a new MotString which represents the content of the span.
         */
        MotString* NewMotString() const;
    };
}

#endif //MOT_SPAN_H
