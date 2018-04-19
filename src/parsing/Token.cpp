#include <cassert>
#include <cstring>
#include <algorithm>
#include "Token.h"
#include "../text/Utf8.h"
#include "Lexer.h"

namespace mot
{
    Token::Token(TokenType type, FileSpan trivia, FileSpan text, MotString value) :
            _type{type},
            _trivia{trivia},
            _text{text},
            _value{value}
    {
        assert(trivia.End() == text.Start());
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

    const MotString& Token::Value() const
    {
        return _value;
    }

    SP<const char> Token::Filename() const
    {
        return _text.Content().Filename();
    }

    void Token::DebugPrint(std::ostream& os, bool positions, bool color) const
    {
        auto reset = color ? "\033[0m" : "";
        auto boldGreen = color ? "\033[1;32m" : "";
        auto boldBlue = color ? "\033[1;34m" : "";
        auto boldRed = color ? "\033[1;31m" : "";
        auto red = color ? "\033[31m" : "";
        auto yellow = color ? "\033[33m" : "";

        auto tokenColor = _type == TokenType::EndOfLine ? boldBlue : boldGreen;
        if (_type == TokenType::Error)
            tokenColor = boldRed;

        auto tokenName = GetTokenTypeName(_type);

        auto start = _text.Start();

        os << tokenColor << tokenName << reset;

        if (positions)
        {
            uint32_t lineNumber, lineStart, column;
            _text.Content().PositionDetails(start, &lineNumber, &lineStart, &column);
            os << lineNumber << ':' << column + 1;
        }

        if (!MotString::IsEmpty(_value))
        {
            if (_type == TokenType::BlockText)
                os << '\n';

            os << yellow << _value;
        }

        os << '\n' << reset;
    }
}
