#include <cassert>
#include "Token.h"


Token::Token(TokenType type, FileSpan trivia, FileSpan text):
        _type(type),
        _trivia(trivia),
        _text(text)
{
    assert(trivia.End() == text.Start());
}

Token::~Token() = default;

bool Token::IsToken() const
{
    return true;
}

TokenType Token::Type() const
{
    return _type;
}

const FileSpan& Token::Trivia() const
{
    return _trivia;
}

const FileSpan& Token::Text() const
{
    return _text;
}

void Token::DebugPrint(FILE* stream, bool positions, bool color) const
{
    auto reset = color ? "\033[0m" : "";
    auto boldGreen = color ? "\033[1;32m" : "";
    auto boldBlue = color ? "\033[1;34m" : "";
    auto boldRed = color ? "\033[1;31m" : "";
    auto yellow = color ? "\033[33m" : "";

    auto tokenColor = _type == TokenType::EndOfLine ? boldBlue : boldGreen;
    if (_type <= TokenType::Error_)
        tokenColor = boldRed;

    auto tokenName = GetTokenTypeName(_type);

    auto start = _text.Start();
    auto end = _text.End();

    fprintf(stream, "%s%s%s ", tokenColor, tokenName, reset);

    if (positions)
    {
        uint32_t lineStart;
        auto lineNumber = LineNumber(lineStart) + 1;
        fprintf(stream, "%u:%u ", lineNumber, start - lineStart + 1);
    }

    if (_text.Length() > 0 && _type != TokenType::EndOfLine)
    {
        fprintf(stream, "%s", yellow);
        _text.Print(stream);
    }

    fprintf(stream, "%s\n", reset);
}

uint32_t Token::LineNumber() const
{
    auto textSpan = Text();
    uint32_t lineStart;
    return textSpan.Content()->LineNumber(textSpan.Start(), lineStart);
}

uint32_t Token::LineNumber(uint32_t& out_lineStart) const
{
    auto textSpan = Text();
    return textSpan.Content()->LineNumber(textSpan.Start(), out_lineStart);
}
