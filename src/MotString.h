#ifndef MOT_GOTSTRING_H
#define MOT_GOTSTRING_H


#include <cstdlib>

class MotString
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
    MotString(char* data, size_t byteCount);
    ~MotString();

    const char* Value() const;
    /**
     * Returns the number of bytes which are meaningful (excludes the null terminator).
     */
    size_t ByteLength() const;
};


#endif //MOT_GOTSTRING_H
