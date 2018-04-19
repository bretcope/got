#ifndef MOT_FMTPOSITION_H
#define MOT_FMTPOSITION_H

#include "../../parsing/Nodes.h"
#include "../../Macros.h"

namespace mot
{
    FLAGS_ENUM(FmtPositionFlags)
    {
        None = 0,

        AtPrefix = 1u << 0u,
        Filename = 1u << 1u,
        Line = 1u << 2u,
        Column = 1u << 3u,
        EndLine = 1u << 4u,

        Default = static_cast<unsigned int>(AtPrefix | Filename | Line | Column | EndLine),
    };

    class FmtPosition
    {
        FileSpan _span;
        FmtPositionFlags _flags;

    public:
        explicit FmtPosition(const Node& node);
        explicit FmtPosition(const Token& token);
        explicit FmtPosition(const FileSpan& span);
        FmtPosition(const FileContent& content, uint32_t position);

        friend std::ostream& operator<<(std::ostream& os, const FmtPosition& s);

        FmtPositionFlags Flags() const;
        FmtPosition& Flags(FmtPositionFlags flags);
        FmtPosition& NoAtPrefix();
        FmtPosition& NoFilename();
        FmtPosition& NoLine();
        FmtPosition& NoColumn();
        FmtPosition& NoEndLine();
    };
}

#endif //MOT_FMTPOSITION_H
