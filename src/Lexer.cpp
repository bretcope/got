#include <cassert>
#include "Lexer.h"

Lexer::Lexer(FileContent* content):
    _content(content),
    _input(content->Data()),
    _size(content->Size())
{
    _content->ResetLineMarkers();
    StartNewLine(0);
}

Lexer::~Lexer()
{
    delete _nextToken;
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
    if (_lastTokenType == TokenType::EndOfInput)
        return nullptr;

    ConsumeTrivia();

    auto pos = _position;

    if (pos >= _size)
        return LexEndOfInput();

    switch (_lastTokenType)
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
                        return NewToken(TokenType::Error_TabIndent, 0);
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
            return LexBlockText();
        case '"':
            return LexQuotedText();
        case '\r':
        case '\n':
            return LexEndOfLine(ch);
        default:
            if (_lastTokenType == TokenType::Colon)
                return LexLineText();

            if (IsAlpha(ch))
                return LexWord();

            return NewToken(TokenType::Error_UnexpectedCharacter, 1);
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

    auto spaces = (int)_lineSpaces;
    auto newLevel = spaces / Lexer::SPACES_PER_INDENT;
    auto diff = newLevel - _indentLevel;

    if (diff < 0)
    {
        _indentLevel--;
        return NewToken(TokenType::Outdent, 0);
    }

    if (diff > 0)
    {
        _indentLevel++;
        return NewToken(TokenType::Indent, 0);
    }

    return NewToken(TokenType::Error_MisalignedIndentation, 0);
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

    return NewToken(TokenType::Word, pos - start);
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

    return NewToken(TokenType::LineText, lastNonWhitespace + 1 - start);
}

Token* Lexer::LexQuotedText()
{
    assert(_input[_position] == '"');

    auto start = _position;
    auto pos = start + 1;
    auto size = _size;
    auto input = _input;

    auto isEscape = false;

    while (pos < size)
    {
        auto ch = input[pos];

        if (IsNewLine(ch))
            break;

        if (isEscape)
        {
            isEscape = false;
        }
        else
        {
            if (ch == '"')
                return NewToken(TokenType::QuotedText, pos + 1 - start);

            if (ch == '\\')
                isEscape = true;
        }

        pos++;
    }

    return NewToken(TokenType::Error_UnterminatedString, pos - start);
}

Token* Lexer::LexBlockText()
{
    assert(_input[_position] == '>');

    auto start = _position;
    auto end = start + 1;
    auto size = _size;
    auto input = _input;

    // advance pos until we get to the end of first line
    while (end < size && !IsNewLine(input[end]))
    {
        end++;
    }

    // figure out which (if any) following lines are part of the block text
    auto requiredSpaces = (_indentLevel + 1) * Lexer::SPACES_PER_INDENT;

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

        if (pos - lineStart < requiredSpaces && pos < size && !IsNewLine(input[pos]))
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

        end = pos;
    }

    return NewToken(TokenType::BlockText, end - start);
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

Token* Lexer::NewToken(TokenType type, uint32_t length)
{
    _lastTokenType = type;
    auto start = _position;
    auto end = start + length;
    _position = end;
    auto token = new Token(type, _trivia, FileSpan(_content, start, end));
    _trivia = {};
    return token;
}
