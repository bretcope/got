#include <cassert>
#include <cstring>
#include <algorithm>
#include "Token.h"
#include "../text/Utf8.h"
#include "Lexer.h"

namespace mot
{
    Token::Token(TokenType type, FileSpan trivia, FileSpan text, MotString* value) :
            _type(type),
            _trivia(trivia),
            _text(text),
            _value(value)
    {
        assert(trivia.End() == text.Start());

#ifndef NDEBUG
        switch (type)
        {
            case TokenType::Word:
            case TokenType::LineText:
            case TokenType::QuotedText:
            case TokenType::BlockText:
                assert(value != nullptr);
                break;
            default:
                assert(value == nullptr);
        }
#endif
    }

    Token::~Token()
    {
        delete _value;
    }

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
            _text.Content()->PositionDetails(start, &lineNumber, &lineStart, &column);
            os << lineNumber << ':' << column + 1;
        }

        if (_value != nullptr)
        {
            if (_type == TokenType::BlockText)
                os << '\n';

            os << yellow << _value;
        }

        os << '\n' << reset;
    }

    const MotString* Token::Value() const
    {
        return _value;
    }

    const char* Token::Filename() const
    {
        return _text.Content()->Filename();
    }
}
