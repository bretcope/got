#ifndef MOT_FILEIR_H
#define MOT_FILEIR_H

#include <cstdio>
#include <vector>
#include "../parsing/Nodes.h"
#include "../text/Collections.h"
#include "../io/Console.h"
#include "../models/OverrideMode.h"

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
        const PropertyNode& Node;
        const MotString Name;
        ByString<MotString> PathByEnvironment;
        mot::OverrideMode OverrideMode;

        PrefixIR(const PropertyNode& node, MotString name);
        PrefixIR(const PrefixIR&) = delete;
        PrefixIR(PrefixIR&& other) noexcept;
    };

    struct RepoIR
    {
        MotString name;
        MotString config;
        ByString<MotString*> remotes;
        StringSet aliases;
        mot::OverrideMode overrideMode;
    };

    class FileIR
    {
    private:
        const Console& _console;
        bool _isValid;
        const FileNode& _fileNode;
        const PropertyNode* _headerNode;
        bool _isProfile = false;
        ByString<UP<AliasIR>> _aliases;
        std::vector<UP<IncludeIR>> _includes;
        ByString<UP<PrefixIR>> _prefixes;
        ByString<UP<RepoIR>> _repos;

    public:
        FileIR(const Console& console, const FileNode& fileNode, bool expectResource);
        FileIR(const FileIR&) = delete;
        FileIR(FileIR&&) = delete;

        bool IsValid() const;

        const ByString<UP<PrefixIR>>& Prefixes() const;

    private:
        bool TryBuildIR(const FileNode& fileNode, bool expectResource);
        bool AddHeader(const PropertyNode& firstNode, bool expectResource);
        bool AddAlias(const PropertyNode& prop);
        bool AddInclude(const PropertyNode& prop);
        bool AddPrefix(const PropertyNode& prop);
        bool AddRepo(const PropertyNode& prop);
    };
}

#endif //MOT_FILEIR_H
