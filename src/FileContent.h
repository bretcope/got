#ifndef MOT_FILECONTENT_H
#define MOT_FILECONTENT_H


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

    /**
     * Returns the number of lines in the file.
     */
    uint32_t LineCount() const;

    /**
     * Returns details about a
     * @param position A byte offset from the start of the file.
     * @param [out] out_lineNumber If non-null, value is set to the line number of the position (first line = 0).
     * @param [out] out_lineStart If non-null, value is set to the position of the start of the line (in bytes, relative to the start of the file).
     * @param [out] out_column If non-null, value is set to the number of unicode characters which come before the current byte position. Since UTF-8
     * characters can span multiple bytes, the column number may be less than the value of (position - out_lineStart).
     */
    void PositionDetails(uint32_t position, uint32_t* out_lineNumber, uint32_t* out_lineStart, uint32_t* out_column) const;

    /**
     * Returns the starting position of a given line. If the line number is greater than the number of lines, the end position of the file is returned.
     */
    uint32_t LineStartPosition(uint32_t lineNumber) const;

    void ResetLineMarkers();
    uint32_t MarkLine(uint32_t position);
};


#endif //MOT_FILECONTENT_H
