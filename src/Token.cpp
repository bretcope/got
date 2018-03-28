#include <cassert>
#include <cstring>
#include "Token.h"
#include "Utf8.h"
#include "Lexer.h"


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
    auto red = color ? "\033[31m" : "";
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

    switch (_type)
    {
        case TokenType::Word:
        case TokenType::LineText:
        case TokenType::QuotedText:
            char* literal;
            fprintf(stream, "%s", red); // in case there's an error
            if (ParseStringValue(stream, &literal))
            {
                fprintf(stream, "%s%s", yellow, literal);
                delete literal;
            }
            break;
        case TokenType::BlockText:
            fprintf(stream, "%s", yellow);
            _text.Print(stream);
            break;
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

bool Token::ParseStringValue(FILE* errStream, char** out_value) const
{
    switch (_type)
    {
        case TokenType::QuotedText:
            return ParseQuotedText(errStream, out_value);
        case TokenType::BlockText:
            return ParseBlockText(errStream, out_value);
        case TokenType::Word:
        case TokenType::LineText:
        {
            *out_value = _text.NewString();
            return true;
        }
        default:
            fprintf(errStream, "Error: ParseStringValue called on token type %s\n", GetTokenTypeName(_type));
            *out_value = nullptr;
            return false;
    }
}

/**
 * Parses an escape sequence starting at the backslash character.
 *
 * @param str The input string.
 * @param index The index of the start of the escape sequence (position of the backslash character).
 * @param length Length of the input string (in bytes).
 * @param [out] out_char The parsed unicode character value.
 * @param [out] out_escapeLength The length of the escape sequence (in bytes, including the backslash character).
 * @return True if the escape sequence was valid.
 */
bool ParseEscapeSequence(const char* str, uint32_t index, uint32_t length, char32_t & out_char, int& out_escapeLength)
{
    assert(length - index > 0);
    assert(str[index] == '\\');

    if (index + 1 >= length)
        return false;

    auto formatChar = (unsigned char)str[index + 1];
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
            // todo: UTF-32
            break;
    }

    return false;
}

bool Token::ParseQuotedText(FILE* errStream, char** out_value) const
{
    assert(_type == TokenType::QuotedText);
    assert(_text.Content()->Data()[_text.Start()] == '"');
    assert(_text.Content()->Data()[_text.End() - 1] == '"');

    auto hasEscapes = false;

    auto start = _text.Start() + 1;
    auto end = _text.End() - 1;

    assert(end > start); // the minimum length of a raw quoted string is 2 (for the open and close quote characters).

    auto data = _text.Content()->Data();
    auto rawLength = end - start;

    // check the string for escape sequences and get the final string length
    auto finalLength = rawLength;
    for (auto i = start; i < end;)
    {
        if (data[i] != '\\')
        {
            i++;
            continue;
        }

        char32_t escapeResult;
        int consumed;
        if (ParseEscapeSequence(data, i, end, escapeResult, consumed))
        {
            i += consumed;
            auto resultLength = Utf8::EncodedSize(escapeResult);
            finalLength += resultLength - consumed;
            hasEscapes = true;
        }
        else
        {
            uint32_t lineStart;
            auto lineNumber = LineNumber(lineStart);
            fprintf(errStream, "Error: Unsupported escape sequence in quoted text\n");
            fprintf(errStream, "    at %s %u:%u\n", _text.Content()->Filename(), lineNumber, i - lineStart + 1);
            return false;
        }
    }

    auto result = new char[finalLength + 1];

    if (hasEscapes)
    {
        // the slow path... we need to loop through looking for escape sequences again
        auto di = start;
        auto ri = 0;
        while (di < end)
        {
            assert(ri < (int)finalLength);

            if (data[di] != '\\')
            {
                result[ri] = data[di];
                di++;
                ri++;
            }
            else
            {
                char32_t escapeResult;
                int consumed;
                ParseEscapeSequence(data, di, end, escapeResult, consumed);
                di += consumed;
                ri += Utf8::Encode(escapeResult, &result[ri]);
            }
        }

        assert (di == end);
        assert (ri == finalLength);
    }
    else
    {
        // no escape sequences, so we can just copy the memory
        memcpy(result, &data[start], rawLength);
    }

    result[finalLength] = '\0';
    *out_value = result;
    return true;
}

bool Token::ParseBlockText(FILE* errStream, char** out_value) const
{
    assert(_type == TokenType::BlockText);

    auto data = _text.Content()->Data();
    auto start = _text.Start();
    auto end = _text.End();

    assert(data[start] == '>');

    // figure out the indent level
//    uint32_t lineStart
//
//    auto spacesCount = _indentLevel * Lexer::SPACES_PER_INDENT;

    // can figure out the number of characters in final string based on the number of lines the block spans


    *out_value = nullptr;
    return false;
}
