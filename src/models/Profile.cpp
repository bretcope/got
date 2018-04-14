#include <cassert>
#include "Profile.h"
#include "../io/FileReader.h"
#include "../ir/ProfileIR.h"
#include "../ir/FileIR.h"

namespace mot
{
    Profile::Profile() = default;

    Profile::~Profile()
    {
        for (auto it = _fileNodes.begin(); it != _fileNodes.end(); ++it)
        {
            delete *it;
        }

        for (auto it = _content.begin(); it != _content.end(); ++it)
        {
            delete *it;
        }
    }

    bool Profile::Load(const Console& console, const char* filename, uint32_t maxFileSize)
    {
        FileContent* profileContent;
        if (!LoadFile(console, filename, maxFileSize, &profileContent))
            return false;

        _content.push_back(profileContent);

        FileNode* profileNode;
        if (!ParseConfigurationFile(console, profileContent, &profileNode))
            return false;

        _fileNodes.push_back(profileNode);

        FileIR profileFileIR(console, profileNode, false);
        if (!profileFileIR.IsValid())
            return false;

        //

        //ProfileIR profileIR;
        //

        return true;
    }
}
