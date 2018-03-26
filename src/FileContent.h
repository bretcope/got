#ifndef GOT_FILECONTENT_H
#define GOT_FILECONTENT_H


#include <cstdint>

class FileContent
{
private:
    char* _filename;
    char* _data;
    uint32_t _size;
    uint32_t* _lineStarts = nullptr;
    uint32_t _lineCount = 0;
    uint32_t _lineCapacity = 0;

public:
    /**
     *
     * @param filename The name of the file. **This string will be owned by the FileContent**, and deleted when FileContent is destructed.
     * @param data Raw binary data which represents the file's content.
     * @param size The size of data (in bytes).
     */
    FileContent(char* filename, char* data, uint32_t size);
    ~FileContent();

    const char* Filename() const;
    const char* Data() const;
    uint32_t Size() const;

    uint32_t LineCount() const;
    void ResetLineMarkers();
    uint32_t MarkLine(uint32_t position);
};


#endif //GOT_FILECONTENT_H
