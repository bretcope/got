#ifndef MOT_TOKEN_H
#define MOT_TOKEN_H

#include <cstdint>
#include <cstdio>
#include <cassert>
#include "../text/MotString.h"
#include "SyntaxElement.h"
#include "../io/FileSpan.h"

namespace mot
{
    enum class TokenType
    {
        Error,

        StartOfInput,

        // Whitespace
                EndOfInput,
        EndOfLine,
        Indent,
        Outdent,

        // Text
                Word,
        LineText,
        QuotedText,
        BlockText,

        // Symbols
        Colon,
        GreaterThan,
    };

    inline const char* GetTokenTypeName(TokenType type)
    {
        switch (type)
        {
            case TokenType::Error:
                return "Error";

            case TokenType::StartOfInput:
                return "StartOfInput";
            case TokenType::EndOfInput:
                return "EndOfInput";
            case TokenType::EndOfLine:
                return "EndOfLine";
            case TokenType::Indent:
                return "Indent";
            case TokenType::Outdent:
                return "Outdent";

            case TokenType::Word:
                return "Word";
            case TokenType::LineText:
                return "LineText";
            case TokenType::QuotedText:
                return "QuotedText";
            case TokenType::BlockText:
                return "BlockText";

            case TokenType::Colon:
                return "Colon";
            case TokenType::GreaterThan:
                return "GreaterThan";
        }

        assert(!"TokenType missing from GetTokenTypeName");
        return "";
    }

    inline std::ostream& operator<<(std::ostream& os, TokenType type)
    {
        os << GetTokenTypeName(type);
        return os;
    }

    class Token final : public SyntaxElement
    {
    private:
        const TokenType _type;
        const FileSpan _trivia;
        const FileSpan _text;
        MotString* _value;

    public:
        Token(TokenType type, FileSpan trivia, FileSpan text, MotString* value = nullptr);
        Token(const Token&) = delete;
        Token(Token&&) = delete;
        ~Token();

        bool IsToken() const final;

        TokenType Type() const;

        const FileSpan& Trivia() const;

        const FileSpan& Text() const;

        void DebugPrint(std::ostream& os, bool positions, bool color) const;

        const MotString* Value() const;

        const char* Filename() const;
    };
}

#endif //MOT_TOKEN_H
