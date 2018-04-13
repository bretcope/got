#include "FmtPosition.h"

namespace mot
{
    FmtPosition::FmtPosition(const Node* node):
            _span(node->Position()),
            _flags(FmtPositionFlags::Default)
    {
    }

    FmtPosition::FmtPosition(const mot::Token* token):
            _span(token->Text()),
            _flags(FmtPositionFlags::Default)
    {
    }

    FmtPosition::FmtPosition(const mot::FileSpan& span):
            _span(span),
            _flags(FmtPositionFlags::Default)
    {
    }

    FmtPosition::FmtPosition(const mot::FileContent* content, uint32_t position):
            _span(content, position, position),
            _flags(FmtPositionFlags::Default)
    {
    }

    std::ostream& operator<<(std::ostream& os, const FmtPosition& fmt)
    {
        auto flags = fmt._flags;
        if ((flags & FmtPositionFlags::AtPrefix) != FmtPositionFlags::None)
        {
            const uint32_t LEN = 6;
            os.write("    at", LEN);
        }

        if ((flags & FmtPositionFlags::Filename) != FmtPositionFlags::None)
        {
            os << " \"" << fmt._span.Content()->Filename() << "\"";
        }

        if ((flags & (FmtPositionFlags::Line | FmtPositionFlags::Column)) != FmtPositionFlags::None)
        {
            uint32_t line, column;
            fmt._span.Content()->PositionDetails(fmt._span.Start(), &line, nullptr, &column);

            os.write(" ", 1);

            if ((flags & FmtPositionFlags::Line) != FmtPositionFlags::None)
                os << "line " << line + 1;

            if ((flags & FmtPositionFlags::Column) != FmtPositionFlags::None)
                os << ':' << column + 1;
        }

        if ((flags & FmtPositionFlags::EndLine) != FmtPositionFlags::None)
            os << '\n';

        return os;
    }

    FmtPositionFlags FmtPosition::Flags() const
    {
        return _flags;
    }

    FmtPosition& FmtPosition::Flags(mot::FmtPositionFlags flags)
    {
        _flags = flags;
        return *this;
    }

    FmtPosition& FmtPosition::NoAtPrefix()
    {
        _flags &= ~FmtPositionFlags::AtPrefix;
        return *this;
    }

    FmtPosition& FmtPosition::NoFilename()
    {
        _flags &= ~FmtPositionFlags::Filename;
        return *this;
    }

    FmtPosition& FmtPosition::NoLine()
    {
        _flags &= ~FmtPositionFlags::Line;
        return *this;
    }

    FmtPosition& FmtPosition::NoColumn()
    {
        _flags &= ~FmtPositionFlags::Column;
        return *this;
    }

    FmtPosition& FmtPosition::NoEndLine()
    {
        _flags &= ~FmtPositionFlags::EndLine;
        return *this;
    }
}
