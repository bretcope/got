#include <cstring>
#include <fstream>
#include <iostream>
#include "FileReader.h"

namespace mot
{
    bool LoadFile(const char* filename, uint32_t maxSize, FILE* errStream, FileContent** out_content)
    {
        *out_content = nullptr;

        if (filename == nullptr)
        {
            fprintf(errStream, "Error: Cannot load file. Filename is null.\n");
            return false;
        }

        auto nameLen = strnlen(filename, MAX_FILENAME_SIZE);

        if (nameLen == 0)
        {
            if (*filename == '\0')
                fprintf(errStream, "Error: Cannot load file. Filename is empty.\n");
            else
                fprintf(errStream, "Error: Cannot load file. Filename is too long.\n");

            return false;
        }

        std::ifstream fs;
        fs.open(filename, std::ifstream::in | std::ifstream::binary | std::ifstream::ate); // start at the end of the file to get its size
        if (!fs.is_open())
        {
            fprintf(errStream, "Error: Unable to open file \"%s\"\n", filename);
            return false;
        }

        auto endPos = fs.tellg();

        if (endPos < 0)
        {
            fprintf(errStream, "Error: Could not read file \"%s\"\n", filename);
            return false;
        }

        if (endPos > maxSize)
        {
            fprintf(errStream, "Error: Cannot load file \"%s\"\n       Its size is larger than %f MB", filename, maxSize / (1024.0 * 1024.0));
            return false;
        }

        auto size = (uint32_t)endPos;
        auto content = new char[size];
        fs.seekg(0, std::ifstream::beg);
        fs.read(content, size);

        if (fs.fail() || fs.tellg() != endPos)
        {
            delete content;
            fprintf(errStream, "Error: Failed while reading file \"%s\"\n", filename);
            return false;
        }

        // make a copy of the filename to be owned by FileContent
        // todo: we should perform some sort of resolution on the file before loading, and we could use that as an opportunity to make a copy of the name
        auto nameCopy = new char[nameLen + 1];
        memcpy(nameCopy, filename, nameLen + 1);

        *out_content = new FileContent(nameCopy, content, size);
        return true;
    }
}
