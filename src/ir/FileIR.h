#ifndef MOT_FILEIR_H
#define MOT_FILEIR_H

#include <cstdio>
#include <vector>
#include "../parsing/Nodes.h"
#include "../text/Collections.h"
#include "../io/Console.h"

namespace mot
{
    struct AliasIR
    {
    };

    struct IncludeIR
    {
    };

    struct PrefixIR
    {
        const PropertyNode* Node;
        const MotString* Name;
        ByStringPtr<const MotString*> PathByEnvironment;
        bool IsOverride;
    };

    struct RepoIR
    {
    };

    class FileIR
    {
    private:
        bool _isValid;
        const FileNode* _fileNode;
        const PropertyNode* _headerNode = nullptr;
        bool _isProfile = false;
        ByStringPtr<AliasIR> _aliases;
        ByStringPtr<FileIR*> _resources;
        std::vector<IncludeIR> _includes;
        ByStringPtr<PrefixIR> _prefixes;
        ByStringPtr<RepoIR> _repos;

    public:
        FileIR(const Console& console, const FileNode* fileNode, bool expectResource);
        FileIR(const FileIR&) = delete;
        FileIR(FileIR&&) = delete;
        ~FileIR();

        bool IsValid() const;

        const ByStringPtr<PrefixIR>& Prefixes() const;

    private:
        bool TryBuildIR(const Console& console, const FileNode* fileNode, bool expectResource);
        bool AddHeader(const Console& console, const PropertyNode* firstNode, bool expectResource);
        bool AddAlias(const Console& console, const PropertyNode* prop);
        bool AddInclude(const Console& console, const PropertyNode* prop);
        bool AddPrefix(const Console& console, const PropertyNode* prop);
        bool AddRepo(const Console& console, const PropertyNode* prop);
    };
}

#endif //MOT_FILEIR_H
