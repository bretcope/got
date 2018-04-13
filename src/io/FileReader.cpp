#include <cstring>
#include <fstream>
#include <iostream>
#include "FileReader.h"

namespace mot
{
    bool LoadFile(const Console& console, const char* filename, uint32_t maxSize, FileContent** out_content)
    {
        *out_content = nullptr;

        if (filename == nullptr)
        {
            console.Error() << "Error: Cannot load file. Filename is null.\n";
            return false;
        }

        auto nameLen = strnlen(filename, MAX_FILENAME_SIZE);

        if (nameLen == 0)
        {
            if (*filename == '\0')
                console.Error() << "Error: Cannot load file. Filename is empty.\n";
            else
                console.Error() << "Error: Cannot load file. Filename is too long.\n";

            return false;
        }

        std::ifstream fs;
        fs.open(filename, std::ifstream::in | std::ifstream::binary | std::ifstream::ate); // start at the end of the file to get its size
        if (!fs.is_open())
        {
            console.Error() << "Error: Unable to open file \"" << filename << "\"\n";
            return false;
        }

        auto endPos = fs.tellg();

        if (endPos < 0)
        {
            console.Error() << "Error: Could not read file \"" << filename << "\"\n";
            return false;
        }

        if (endPos > maxSize)
        {
            console.Error() << "Error: Cannot load file \"" << filename << "\"\n";
            console.Error() << "       Its size is larger than " << maxSize / (1024.0 * 1024.0) << "MB\n";
            return false;
        }

        auto size = (uint32_t)endPos;
        auto content = new char[size];
        fs.seekg(0, std::ifstream::beg);
        fs.read(content, size);

        if (fs.fail() || fs.tellg() != endPos)
        {
            delete content;
            console.Error() << "Error: Failed while reading file \"" << filename << "\"\n";
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
