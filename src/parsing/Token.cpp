#include <cassert>
#include <cstring>
#include <algorithm>
#include "Token.h"
#include "../io/Utf8.h"
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
        // todo: use out_column param
        uint32_t lineNumber, lineStart;
        _text.Content()->PositionDetails(_text.Start(), &lineNumber, &lineStart, nullptr);
        fprintf(stream, "%u:%u ", lineNumber, start - lineStart + 1);
    }

    switch (_type)
    {
        case TokenType::BlockText:
            fprintf(stream, "\n");
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
    }

    fprintf(stream, "%s\n", reset);
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
            uint32_t lineNumber, lineStart;
            _text.Content()->PositionDetails(_text.Start(), &lineNumber, &lineStart, nullptr);
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

    auto content = _text.Content();
    auto data = content->Data();
    auto rawStart = _text.Start();
    auto end = _text.End();

    if (rawStart == end)
    {
        *out_value = new char[1] { '\0' };
        return true;
    }

    assert(data[rawStart] == '\r' || data[rawStart] == '\n');

    // figure out the indent level
    uint32_t propertyLineNumber, propertyLineStart;
    content->PositionDetails(rawStart, &propertyLineNumber, &propertyLineStart, nullptr);
    auto spacesCount = 0u;
    while (propertyLineStart + spacesCount < rawStart && data[propertyLineStart + spacesCount] == ' ')
    {
        spacesCount++;
    }

    assert((spacesCount % Lexer::SPACES_PER_INDENT) == 0); // the lexer is supposed to emit errors for invalid indentation alignments

    spacesCount += Lexer::SPACES_PER_INDENT; // add an additional level of indentation

    auto currLine = propertyLineNumber + 1;
    auto currStart = content->LineStartPosition(currLine);

    assert(currStart > rawStart);

    // allocate a new string which includes enough room for all the indentation. This wastes a little bit of memory, but means we don't have to iterate
    // through the lines of text twice.
    auto resultCapacity = end - currStart + 1;
    auto result = new char[resultCapacity];
    auto ri = 0u;

    while (currStart < end)
    {
        auto nextStart = content->LineStartPosition(currLine + 1);
        assert(nextStart > currStart);

        uint32_t copyCount;
        uint32_t copyStart;
        if (nextStart < end)
        {
            // this is not the final line of the block, so there's a line terminator we care about - let's find out how long it is
            auto terminatorLength = data[nextStart - 1] == '\n' && nextStart - 2 >= currStart && data[nextStart - 2] == '\r' ? 2 : 1;
            auto lineContentLength = nextStart - currStart - terminatorLength;
            copyCount = terminatorLength + (lineContentLength > spacesCount ? lineContentLength - spacesCount : 0);
            copyStart = nextStart - copyCount;
        }
        else
        {
            // this is the final line of the block, so we don't care about the line terminator
            auto lineContentLength = end - currStart;
            copyCount = lineContentLength > spacesCount ? lineContentLength - spacesCount : 0;
            copyStart = end - copyCount;
        }

        if (copyCount > 0)
        {
            assert(ri + copyCount <= resultCapacity);
            assert(copyStart + copyCount <= content->Size());
            assert(copyStart + copyCount <= end);
            memcpy(&result[ri], &data[copyStart], copyCount);
        }

        ri += copyCount;
        currLine++;
        currStart = nextStart;
    }

    assert(ri < resultCapacity);

    result[ri] = '\0';
    *out_value = result;
    return true;
}
