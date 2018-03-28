#ifndef MOT_SPAN_H
#define MOT_SPAN_H

#include <cstdint>
#include <cstdio>
#include "FileContent.h"

class FileSpan
{
private:
    const FileContent* _content;
    uint32_t _start;
    uint32_t _end;

public:
    FileSpan(const FileContent* content, uint32_t start, uint32_t end);
    ~FileSpan();

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
     * Prints the span's characters to the stream.
     * @return The number of bytes printed (equal to Length()).
     */
    size_t Print(FILE* stream) const;

    /**
     * Allocates a new null-terminated string which represents filled with the content of the span.
     */
    char* NewString() const;
};

#endif //MOT_SPAN_H
