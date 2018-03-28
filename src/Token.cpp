#include <cassert>
#include "Token.h"


Token::Token(TokenType type, FileSpan triviaPosition, FileSpan position):
        Type(type),
        TriviaPosition(triviaPosition),
        Position(position)
{
    assert(triviaPosition.End() == position.Start());
}

Token::~Token() = default;

bool Token::IsToken() const
{
    return true;
}

void Token::DebugPrint(FILE* stream, bool positions, bool color) const
{
    auto reset = color ? "\033[0m" : "";
    auto boldGreen = color ? "\033[1;32m" : "";
    auto boldBlue = color ? "\033[1;34m" : "";
    auto boldRed = color ? "\033[1;31m" : "";
    auto yellow = color ? "\033[33m" : "";

    auto tokenColor = Type == TokenType::EndOfLine ? boldBlue : boldGreen;
    if (Type <= TokenType::Error_)
        tokenColor = boldRed;

    auto tokenName = GetTokenTypeName(Type);

    auto start = Position.Start();
    auto end = Position.End();

    fprintf(stream, "%s%s%s ", tokenColor, tokenName, reset);

    if (positions)
    {
        fprintf(stream, "%u:%u ", start, end);
    }

    if (Position.Length() > 0 && Type != TokenType::EndOfLine)
    {
        fprintf(stream, "%s", yellow);
        Position.Print(stream);
    }

    fprintf(stream, "%s\n", reset);
}
