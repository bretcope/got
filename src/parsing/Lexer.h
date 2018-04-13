#ifndef MOT_LEXER_H
#define MOT_LEXER_H


#include <cstddef>
#include <cstdint>
#include "Token.h"
#include "../io/FileSpan.h"
#include "../io/Console.h"

namespace mot
{
    class Lexer
    {
    private:
        const Console& _console;    ///< Stream where error messages will be printed.
        FileContent* _content;      ///< The file content to be lexed.
        const char* _input;         ///< Raw binary representation of the file to lex (copy of _content->Data()). Encoding is expected to be UTF-8.
        uint32_t _position = 0;     ///< Current byte position in the input;
        uint32_t _size;             ///< The size (in bytes) of the input (copy of _content->Data()).
        uint32_t _indentLevel = 0;  ///< Number of indent tokens which have been emitted, minus the number of outdent tokens emitted.
        uint32_t _lineStart;        ///< Byte position of the first character of the current line.
        uint32_t _lineSpaces;       ///< The number of spaces before first non-space character on the current line. Used to detect indentation levels.

        Token* _nextToken = nullptr;                        ///< Next unconsumed token, stored here for peek operations.
        FileSpan _trivia;                                   ///< Span of leading trivia which will be attached to the next lexed token.
        TokenType _lastTokenType = TokenType::StartOfInput; ///< The last token type lexed.

    public:
        static const uint32_t SPACES_PER_INDENT = 4;

        Lexer(const Console& console, FileContent* content);
        Lexer(const Lexer&) = delete;
        Lexer(Lexer&&) = delete;
        ~Lexer();

        TokenType PeekType();

        Token* Peek();

        Token* Advance();

        bool Consume(TokenType type, Token** out_token);

    private:
        Token* Lex();

        void ConsumeTrivia();

        Token* LexEndOfInput();

        Token* LexIndentation();

        Token* LexEndOfLine(char currentChar);

        Token* LexWord();

        Token* LexLineText();

        Token* LexQuotedText();

        Token* LexBlockText();

        /**
         * Parses an escape sequence starting at the backslash character.
         *
         * @param position The index of the start of the escape sequence (position of the backslash character).
         * @param [out] out_char The parsed unicode character value.
         * @param [out] out_escapeLength The length of the escape sequence (in bytes, including the backslash character).
         * @return True if the escape sequence was valid.
         */
        bool ParseEscapeSequence(uint32_t position, char32_t& out_char, int& out_escapeLength);

        MotString* GetQuotedLiteral(uint32_t openQuote, uint32_t closeQuote, uint32_t resultLength, bool hasEscapes);

        /**
         * Denotes the start of a new line (does not update _position).
         * @param lineStart The position of the start of the line.
         * @return The number of spaces at the start of the new line.
         */
        uint32_t StartNewLine(uint32_t lineStart);

        Token* NewToken(TokenType type, uint32_t length, MotString* value = nullptr);
    };
}


#endif //MOT_LEXER_H
