#include <algorithm>
#include <cassert>
#include <cstring>
#include <vector>
#include "Lexer.h"
#include "../io/Utf8.h"

Lexer::Lexer(FileContent* content, FILE* errStream):
    _errStream(errStream),
    _content(content),
    _input(content->Data()),
    _size(content->Size()),
    _trivia(content, 0, 0)
{
    _content->ResetLineMarkers();
    StartNewLine(0);
}

Lexer::~Lexer()
{
    delete _nextToken;
}

TokenType Lexer::PeekType()
{
    auto token = Peek();
    return token == nullptr ? TokenType::Error_ : token->Type();
}

Token* Lexer::Peek()
{
    auto next = _nextToken;
    if (next == nullptr)
    {
        next = Lex();
        _nextToken = next;
    }

    return next;
}

Token* Lexer::Advance()
{
    auto next = _nextToken;
    if (next == nullptr)
    {
        return Lex();
    }

    _nextToken = nullptr;
    return next;
}

bool Lexer::Consume(TokenType type, Token** out_token)
{
    auto token = Peek();
    if (token != nullptr && token->Type() == type)
    {
        *out_token = Advance();
        return true;
    }

    *out_token = nullptr;
    return false;
}

static inline bool IsAlpha(char ch)
{
    return ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z' || ch == '_';
}

static inline bool IsAlphaNumeric(char ch)
{
    return IsAlpha(ch) || ch >= '0' && ch <= '9';
}

static inline bool IsNewLine(char ch)
{
    return ch == '\r' || ch == '\n';
}

Token* Lexer::Lex()
{
    auto lastTokenType = _lastTokenType;

    if (lastTokenType == TokenType::EndOfInput)
        return nullptr;

    ConsumeTrivia();

    auto pos = _position;

    if (pos >= _size)
        return LexEndOfInput();

    switch (lastTokenType)
    {
        case TokenType::StartOfInput:
        case TokenType::EndOfLine:
        case TokenType::Indent:
        case TokenType::Outdent:
        {
            if (_position != _lineStart + _lineSpaces)
            {
                // the whitespace consumed at the start of the line doesn't match the number of spaces at the start of the line, which means there might be a
                // tab character.
                for (auto i = _lineStart; i < pos; i++)
                {
                    if (_input[i] == '\t')
                        return NewToken(TokenType::Error_TabIndent, 0); // todo: error message
                }
            }

            // We only need to check for indentation changes immediately after the above tokens
            if (_indentLevel * Lexer::SPACES_PER_INDENT != _lineSpaces)
            {
                return LexIndentation();
            }
            break;
        }
    }

    auto ch = _input[pos];

    switch (ch)
    {
        case ':':
            return NewToken(TokenType::Colon, 1);
        case '>':
            return NewToken(TokenType::GreaterThan, 1);
        case '"':
            return LexQuotedText();
        case '\r':
        case '\n':
            if (lastTokenType == TokenType::GreaterThan)
                return LexBlockText();

            return LexEndOfLine(ch);
        default:
            if (lastTokenType == TokenType::Colon)
                return LexLineText();

            if (IsAlpha(ch))
                return LexWord();

            return NewToken(TokenType::Error_UnexpectedCharacter, 1); // todo: error message
    }
}

void Lexer::ConsumeTrivia()
{
    auto input = _input;
    auto pos = _position;
    auto size = _size;
    auto isComment = false;

    while (pos < size)
    {
        auto ch = input[pos];
        switch (ch)
        {
            case ' ':
            case '\t':
                pos++;
                continue;
            case '\r':
            case '\n':
            {
                auto lastTokenType = _lastTokenType;
                if (lastTokenType == TokenType::EndOfLine || lastTokenType == TokenType::StartOfInput || lastTokenType == TokenType::Outdent)
                {
                    // The lexer isn't supposed to emit an EndOfLine token immediately after a StartOfInput, EndOfLine, or Outdent token.
                    pos += (ch == '\r' && pos + 1 < size && input[pos + 1] == '\n') ? 2 : 1;
                    pos += StartNewLine(pos);
                    isComment = false;
                    break;
                }
                goto DONE; // last token wasn't a StartOfInput, EndOfLine, or Outdent, so this new line is significant (don't consume it as trivia).
            }
            case '#':
                isComment = true;
                pos++;
                continue;
            default:
                if (isComment)
                {
                    pos++;
                    continue;
                }
                goto DONE;
        }
    }

    DONE:
    _trivia = FileSpan(_content, _position, pos);
    _position = pos;
}

Token* Lexer::LexEndOfInput()
{
    assert(_position == _size);

    auto lastTokenType = _lastTokenType;
    if (lastTokenType == TokenType::GreaterThan)
        return LexBlockText();

    if (lastTokenType == TokenType::EndOfLine || lastTokenType == TokenType::Outdent)
    {
        if (_indentLevel > 0)
        {
            _indentLevel--;
            return NewToken(TokenType::Outdent, 0);
        }

        return NewToken(TokenType::EndOfInput, 0);
    }

    return NewToken(TokenType::EndOfLine, 0);
}

Token* Lexer::LexIndentation()
{
    assert(_lineSpaces != _indentLevel * Lexer::SPACES_PER_INDENT);

    auto newLevel = _lineSpaces / Lexer::SPACES_PER_INDENT;
    auto diff = (int32_t)newLevel - (int32_t)_indentLevel;

    if (diff < 0)
    {
        assert(_indentLevel > 0);
        _indentLevel--;
        return NewToken(TokenType::Outdent, 0);
    }

    if (diff > 0)
    {
        _indentLevel++;
        return NewToken(TokenType::Indent, 0);
    }

    return NewToken(TokenType::Error_MisalignedIndentation, 0); // todo: error message
}

Token* Lexer::LexEndOfLine(char currentChar)
{
    assert(IsNewLine(currentChar));
    assert(currentChar == _input[_position]);

    auto length = 1u;
    if (currentChar == '\r')
    {
        auto pos = _position;
        if (pos + 1 < _size && _input[pos + 1] == '\n')
            length = 2;
    }

    auto token = NewToken(TokenType::EndOfLine, length);
    StartNewLine(_position);
    return token;
}

Token* Lexer::LexWord()
{
    assert(IsAlpha(_input[_position]));

    auto start = _position;
    auto pos = start + 1;
    auto size = _size;
    auto input = _input;

    while (pos < size)
    {
        auto ch = input[pos];

        if (!IsAlphaNumeric(ch))
            break;

        pos++;
    }

    auto length = pos - start;
    auto value = new MotString(&input[start], length, false);
    return NewToken(TokenType::Word, length, value);
}

Token* Lexer::LexLineText()
{
    auto start = _position;
    auto pos = start + 1;
    auto size = _size;
    auto input = _input;
    auto lastNonWhitespace = start;

    while (pos < size)
    {
        auto ch = input[pos];

        if (IsNewLine(ch) || ch == '#')
            break;

        if (ch != ' ' && ch != '\t')
            lastNonWhitespace = pos;

        pos++;
    }

    auto length = lastNonWhitespace + 1 - start;
    auto value = new MotString(&input[start], length, false);
    return NewToken(TokenType::LineText, length, value);
}

Token* Lexer::LexQuotedText()
{
    assert(_input[_position] == '"');

    auto start = _position;
    auto pos = start + 1;
    auto size = _size;
    auto input = _input;

    auto hasEscapes = false;
    auto resultLength = 0u;

    while (pos < size)
    {
        auto ch = input[pos];

        if (IsNewLine(ch))
            break;

        if (ch == '"')
        {
            auto value = GetQuotedLiteral(start, pos, resultLength, hasEscapes);
            return NewToken(TokenType::QuotedText, pos + 1 - start, value);
        }

        if (ch != '\\')
        {
            pos++;
            resultLength++;
            continue;
        }

        char32_t escapeResult;
        int consumed;
        if (ParseEscapeSequence(pos, escapeResult, consumed))
        {
            pos += consumed;
            resultLength += Utf8::EncodedSize(escapeResult);
            hasEscapes = true;
        }
        else
        {
            uint32_t lineNumber, lineStart;
            _content->PositionDetails(start, &lineNumber, &lineStart, nullptr);
            fprintf(_errStream, "Error: Unsupported escape sequence in quoted text\n");
            fprintf(_errStream, "    at %s %u:%u\n", _content->Filename(), lineNumber, pos - lineStart + 1);
            return NewToken(TokenType::Error_, pos - start);
        }
    }

    return NewToken(TokenType::Error_UnterminatedString, pos - start); // todo: error message

}

MotString* Lexer::GetQuotedLiteral(uint32_t openQuote, uint32_t closeQuote, uint32_t resultLength, bool hasEscapes)
{
    assert(closeQuote > openQuote);

    if (closeQuote - openQuote == 1)
        new MotString(nullptr, 0, false);

    auto result = new char[resultLength];
    auto input = _input;

    if (hasEscapes)
    {
        // the slow path... we need to loop through looking for escape sequences again
        auto ii = openQuote + 1; // start copying after the first double quote
        auto ri = 0u;
        while (ii < closeQuote)
        {
            assert(ri < (int)resultLength);

            if (input[ii] != '\\')
            {
                result[ri] = input[ii];
                ii++;
                ri++;
            }
            else
            {
                char32_t escapeResult;
                int consumed;
                ParseEscapeSequence(ii, escapeResult, consumed);
                ii += consumed;
                ri += Utf8::Encode(escapeResult, &result[ri]);
            }
        }

        assert (ii == closeQuote);
        assert (ri == resultLength);
    }
    else
    {
        // no escape sequences, so we can just copy the memory
        memcpy(result, &input[openQuote + 1], closeQuote - openQuote - 1);
    }

    return new MotString(result, resultLength, true);
}

bool Lexer::ParseEscapeSequence(uint32_t position, char32_t& out_char, int& out_escapeLength)
{
    auto size = _size;
    auto input = _input;

    assert(position < size);
    assert(input[position] == '\\');

    if (position + 1 >= size)
        return false;

    auto formatChar = (unsigned char)input[position + 1];
    switch (formatChar)
    {
        case '\'':
        case '"':
        case '?':
        case '\\':
            // simple case where the escape sequence just represents the literal value of the second character
            out_char = formatChar;
            out_escapeLength = 2;
            return true;
        case 'a':
            out_char = '\a';
            out_escapeLength = 2;
            return true;
        case 'b':
            out_char = '\b';
            out_escapeLength = 2;
            return true;
        case 'f':
            out_char = '\f';
            out_escapeLength = 2;
            return true;
        case 'n':
            out_char = '\n';
            out_escapeLength = 2;
            return true;
        case 'r':
            out_char = '\r';
            out_escapeLength = 2;
            return true;
        case 't':
            out_char = '\t';
            out_escapeLength = 2;
            return true;
        case 'v':
            out_char = '\v';
            out_escapeLength = 2;
            return true;
        case 'u':
        {
            auto digits = 0;
            char32_t value = 0u;
            for (auto i = position + 2; digits < 6 && i < size; i++, digits++)
            {
                auto ch = input[i];
                if (ch >= '0' && ch <= '9')
                {
                    value = (value << 4) | (ch - 0x30u);
                    continue;
                }

                ch &= ~0x20u; // to uppercase
                if (ch >= 'A' && ch <= 'F')
                {
                    value = (value << 4) | (ch - 'A' + 10);
                    continue;
                }

                break;
            }

            out_char = value;
            out_escapeLength = 2 + digits;
            return digits > 0;
        }
    }

    return false;
}

Token* Lexer::LexBlockText()
{
    assert(_position == _size || _input[_position] == '\r' || _input[_position] == '\n');

    auto start = _position;
    auto end = start;
    auto size = _size;
    auto input = _input;

    // figure out which (if any) following lines are part of the block text
    auto requiredSpaces = (_indentLevel + 1) * Lexer::SPACES_PER_INDENT;
    auto valueLength = 0u;

    struct CopyRange { uint32_t start, end; };
    std::vector<CopyRange> copyRanges;

    while (end < size)
    {
        // check if the next line is part of the block text
        // start by advancing past the new line
        auto lineStart = end + (input[end] == '\r' && end + 1 < size && input[end + 1] == '\n' ? 2 : 1);

        auto pos = lineStart;
        while (pos < size && input[pos] == ' ')
        {
            pos++;
        }

        auto padding = std::min(pos - lineStart, requiredSpaces);

        if (padding < requiredSpaces && pos < size && !IsNewLine(input[pos]))
        {
            // We didn't reach the required number of spaces, and we're not at the end of the line.
            // This means "end" already points to the end of the block text.
            break;
        }

        // Now we're sure the previous new-line was part of this block text, so let's consume it.
        StartNewLine(lineStart);

        // find the end of this line so we can start over
        while (pos < size && !IsNewLine(input[pos]))
        {
            pos++;
        }

        if (copyRanges.size() == 0)
        {
            valueLength = pos - (lineStart + padding);
            copyRanges.push_back(CopyRange{lineStart + padding, pos});
        }
        else
        {
            // make the last range include the new line sequence
            assert(copyRanges.back().end == end);
            valueLength += lineStart - end;
            copyRanges.back().end = lineStart;

            if (pos > lineStart)
            {
                valueLength += pos - (lineStart + padding);
                copyRanges.push_back(CopyRange{lineStart + padding, pos});
            }
        }

        end = pos;
    }

    MotString* value;
    if (valueLength == 0)
    {
        value = new MotString(nullptr, 0, false);
    }
    else
    {
        auto vi = 0u;
        auto valueStr = new char[valueLength];

        for (auto it = copyRanges.begin(); it != copyRanges.end(); ++it)
        {
            auto rs = it->start;
            auto length = it->end - rs;
            if (length > 0)
            {
                memcpy(&valueStr[vi], &input[rs], length);
            }

            vi += length;
        }

        assert(vi == valueLength);
        value = new MotString(valueStr, valueLength, true);
    }

    return NewToken(TokenType::BlockText, end - start, value);
}

uint32_t Lexer::StartNewLine(uint32_t lineStart)
{
    _content->MarkLine(lineStart);

    auto pos = lineStart;
    auto size = _size;
    auto input = _input;
    while (pos < size && input[pos] == ' ')
    {
        pos++;
    }

    _lineStart = lineStart;

    auto spaces = pos - lineStart;
    _lineSpaces = spaces;

    return spaces;
}

Token* Lexer::NewToken(TokenType type, uint32_t length, MotString* value)
{
    _lastTokenType = type;
    auto start = _position;
    auto end = start + length;
    _position = end;
    auto token = new Token(type, _trivia, FileSpan(_content, start, end), value);
    return token;
}
