#ifndef GOT_TOKEN_H
#define GOT_TOKEN_H

#include <cstdint>
#include <cstdio>
#include "GotString.h"
#include "SyntaxElement.h"

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
};

inline const char* GetTokenTypeName(TokenType type)
{
    switch (type)
    {
        case TokenType::Error_UnexpectedCharacter: return "Error_UnexpectedCharacter";
        case TokenType::Error_MisalignedIndentation: return "Error_MisalignedIndentation";
        case TokenType::Error_UnterminatedString: return "Error_UnterminatedString";
        case TokenType::Error_TabIndent: return "Error_TabIndent";
        case TokenType::Error_: return "Error_";

        case TokenType::StartOfInput: return "StartOfInput";
        case TokenType::EndOfInput: return "EndOfInput";
        case TokenType::EndOfLine: return "EndOfLine";
        case TokenType::Indent: return "Indent";
        case TokenType::Outdent: return "Outdent";

        case TokenType::Word: return "Word";
        case TokenType::LineText: return "LineText";
        case TokenType::QuotedText: return "QuotedText";
        case TokenType::BlockText: return "BlockText";

        case TokenType::Colon: return "Colon";
    }

    return "";
}

struct Span
{
    /**
     * Inclusive Start position (byte offset from the document's start).
     */
    uint32_t Start;
    /**
     * Exclusive End position (byte offset from the document's start).
     */
    uint32_t End;

    Span()
    {
        Start = 0;
        End = 0;
    }

    Span(uint32_t start, uint32_t end)
    {
        Start = start;
        End = end;
    }
};

class Token final : public SyntaxElement
{
public:
    const TokenType Type;
    const Span TriviaPosition;
    const Span Position;

    Token(TokenType type, Span triviaPosition, Span position);
    ~Token();

    bool IsToken() const final;

    void DebugPrint(FILE* stream, const char* content, bool positions, bool color) const;

    // todo: char* ParseStringValue
};


#endif //GOT_TOKEN_H
