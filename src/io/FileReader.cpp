#include <cstring>
#include <fstream>
#include <iostream>
#include "FileReader.h"

namespace mot
{
    UP<FileContent> LoadFile(const Console& console, const char* filename, uint32_t maxSize)
    {
        if (filename == nullptr)
        {
            console.Error() << "Error: Cannot load file. Filename is null.\n";
            return nullptr;
        }

        auto nameLen = strnlen(filename, MAX_FILENAME_SIZE);

        if (nameLen == 0)
        {
            if (*filename == '\0')
                console.Error() << "Error: Cannot load file. Filename is empty.\n";
            else
                console.Error() << "Error: Cannot load file. Filename is too long.\n";

            return nullptr;
        }

        std::ifstream fs;
        fs.open(filename, std::ifstream::in | std::ifstream::binary | std::ifstream::ate); // start at the end of the file to get its size
        if (!fs.is_open())
        {
            console.Error() << "Error: Unable to open file \"" << filename << "\"\n";
            return nullptr;
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
        UP<char[]> data{new char[size]};
        fs.seekg(0, std::ifstream::beg);
        fs.read(data.get(), size);

        if (fs.fail() || fs.tellg() != endPos)
        {
            console.Error() << "Error: Failed while reading file \"" << filename << "\"\n";
            return nullptr;
        }

        // make a copy of the filename to be owned by FileContent
        // todo: we should perform some sort of resolution on the file before loading, and we could use that as an opportunity to make a copy of the name
        UP<char[]> nameCopy{new char[nameLen + 1]};
        memcpy(nameCopy.get(), filename, nameLen + 1);

        return UP<FileContent>{new FileContent(nameCopy.release(), data.release(), size)};
    }
}
