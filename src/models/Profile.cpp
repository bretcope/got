#include <cassert>
#include "Profile.h"
#include "../io/FileReader.h"
#include "../ir/ProfileIR.h"
#include "../ir/FileIR.h"

namespace mot
{
    Profile::Profile() = default;

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

        FileNode* profileNodePtr;

        {
            auto profileNode = ParseConfigurationFile(console, *contentPtr);
            if (profileNode == nullptr)
                return false;

            profileNodePtr = profileNode.get();
            _fileNodes.push_back(std::move(profileNode));
        }

        FileIR profileFileIR(console, *profileNodePtr, false);
        if (!profileFileIR.IsValid())
            return false;

        //

        //ProfileIR profileIR;
        //

        return true;
    }
}
