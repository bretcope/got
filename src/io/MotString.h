#ifndef MOT_GOTSTRING_H
#define MOT_GOTSTRING_H


#include <cstdlib>
#include <cstdint>
#include <cstdio>

class MotString
{
private:
    const char* _data;
    uint32_t _byteCount;
    mutable uint32_t _hashCode = 0; // lazy loaded
    bool _isOwnerOfData;

public:
    /**
     *
     * @param data The raw UTF-8 string.
     * @param byteCount The number of bytes which are meaningful (excludes the null terminator).
     * @param transferOwnership If true, the data will be deallocated when the MotString is destructed.
     */
    MotString(const char* data, uint32_t byteCount, bool transferOwnership);
    ~MotString();

    static MotString* NewFromConstant(const char* str);

    /**
     * Returns the number of bytes which are meaningful (excludes the null terminator).
     */
    uint32_t ByteLength() const;

    /**
     * Returns a case-insensitive hash code for the string.
     */
    uint32_t HashCode() const;

    /**
     * Prints the string to a file stream.
     */
    void Print(FILE* stream) const;
};


#endif //MOT_GOTSTRING_H
