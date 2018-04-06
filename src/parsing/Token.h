#ifndef MOT_TOKEN_H
#define MOT_TOKEN_H

#include <cstdint>
#include <cstdio>
#include "../io/MotString.h"
#include "SyntaxElement.h"
#include "../io/FileSpan.h"

namespace mot
{
    enum class TokenType
    {
        Error_UnexpectedCharacter,
        Error_MisalignedIndentation,
        Error_UnterminatedString,
        Error_TabIndent,
        Error_, // all types less than this must be error tokens. This value gives a reference for doing comparisons (e.g. type <= Error_).

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
            case TokenType::Error_UnexpectedCharacter:
                return "Error_UnexpectedCharacter";
            case TokenType::Error_MisalignedIndentation:
                return "Error_MisalignedIndentation";
            case TokenType::Error_UnterminatedString:
                return "Error_UnterminatedString";
            case TokenType::Error_TabIndent:
                return "Error_TabIndent";
            case TokenType::Error_:
                return "Error_";

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

        return "";
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

        ~Token();

        bool IsToken() const final;

        TokenType Type() const;

        const FileSpan& Trivia() const;

        const FileSpan& Text() const;

        void DebugPrint(FILE* stream, bool positions, bool color) const;

        const MotString* Value() const;
    };
}

#endif //MOT_TOKEN_H
