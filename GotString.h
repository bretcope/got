#ifndef GOT_GOTSTRING_H
#define GOT_GOTSTRING_H


#include <cstdlib>

class GotString
{
private:
    char* _data;
    size_t _byteCount;

public:
    /**
     *
     * @param data The raw UTF-8 string.
     * @param byteCount The number of bytes which are meaningful (excludes the null terminator).
     */
    GotString(char* data, size_t byteCount);
    ~GotString();

    const char* Value() const;
    /**
     * Returns the number of bytes which are meaningful (excludes the null terminator).
     */
    size_t ByteLength() const;
};


#endif //GOT_GOTSTRING_H
