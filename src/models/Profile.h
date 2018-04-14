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
        const MotString* _name = nullptr;
        ByStringPtr<const Prefix> _prefixes;
        ByStringPtr<const Repo> _repos;
        std::vector<UP<FileContent>> _content;
        std::vector<FileNode*> _fileNodes;

    public:
        Profile();
        ~Profile();

        bool Load(const Console& console, const char* filename, uint32_t maxFileSize);
    };
}

#endif //MOT_PROFILE_H
