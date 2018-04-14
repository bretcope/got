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
    }

    bool Profile::Load(const Console& console, const char* filename, uint32_t maxFileSize)
    {
        FileContent* contentPtr;

        {
            auto profileContent = LoadFile(console, filename, maxFileSize);
            if (profileContent == nullptr)
                return false;

            contentPtr = profileContent.get();
            _content.push_back(std::move(profileContent));
        }

        FileNode* profileNode;
        if (!ParseConfigurationFile(console, contentPtr, &profileNode))
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
