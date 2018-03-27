#ifndef MOT_SPAN_H
#define MOT_SPAN_H

#include <cstdint>
#include "FileContent.h"

class FileSpan
{
private:
    const FileContent* _content;
    uint32_t _start;
    uint32_t _end;

public:
    FileSpan();
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
};

#endif //MOT_SPAN_H
