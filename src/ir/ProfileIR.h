#ifndef MOT_PROFILEIR_H
#define MOT_PROFILEIR_H

#include "../parsing/Nodes.h"
#include "../io/Console.h"

namespace mot
{
    class ProfileIR
    {
    private:
        FileNode* _profileFile = nullptr;


    public:
        ProfileIR();
        ~ProfileIR();

        bool IsValid() const;

        bool SetProfileFile(const Console& console, const FileNode* fileNode);
    };
}

#endif //MOT_PROFILEIR_H
