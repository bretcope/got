#ifndef MOT_PROFILE_H
#define MOT_PROFILE_H

#include <cstdio>
#include <vector>
#include "../parsing/Parser.h"
#include "Prefix.h"
#include "../text/Collections.h"
#include "Repo.h"
#include "../io/Console.h"
#include "../Common.h"

namespace mot
{
    class Profile
    {
    private:
        MotString _name;
        ByString<const Prefix> _prefixes;
        ByString<const Repo> _repos;
        std::vector<UP<FileContent>> _content;
        std::vector<UP<FileNode>> _fileNodes;

    public:
        Profile();

        bool Load(const Console& console, const char* filename, uint32_t maxFileSize);
    };
}

#endif //MOT_PROFILE_H
