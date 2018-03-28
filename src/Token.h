#ifndef MOT_TOKEN_H
#define MOT_TOKEN_H

#include <cstdint>
#include <cstdio>
#include "MotString.h"
#include "SyntaxElement.h"
#include "FileSpan.h"

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

class Token final : public SyntaxElement
{
public:
    const TokenType Type;
    const FileSpan TriviaPosition;
    const FileSpan Position;

    Token(TokenType type, FileSpan triviaPosition, FileSpan position);
    ~Token();

    bool IsToken() const final;

    void DebugPrint(FILE* stream, bool positions, bool color) const;

    // todo: char* ParseStringValue
};


#endif //MOT_TOKEN_H
