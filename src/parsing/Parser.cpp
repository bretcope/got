#include <cstring>
#include <algorithm>
#include "Parser.h"
#include "Lexer.h"
#include "../io/printing/FmtPosition.h"

namespace mot
{
    class Parser
    {
        const Console& _console;
        Lexer& _lexer;
        FileContent& _content;

    public:
        Parser(const Console& console, Lexer& lexer, FileContent& content);
        Parser(const Parser&) = delete;
        Parser(Parser&&) = delete;

        UP<FileNode> ParseFile();

        UP<PropertyListNode> ParsePropertyList();

        UP<PropertyNode> ParseProperty();

        UP<PropertyDeclarationNode> ParsePropertyDeclaration();

        UP<PropertyValueNode> ParsePropertyValue();

        UP<PropertyBlockNode> ParsePropertyBlock();

        UP<Token> RequireToken(TokenType type);

        void UnexpectedToken(TokenType expected);

        void UnexpectedToken(TokenType expected0, TokenType expected1);

        void UnexpectedToken(int expectedCount, TokenType* expectedList);
    };

    Parser::Parser(const Console& console, Lexer& lexer, FileContent& content) :
            _console(console),
            _lexer(lexer),
            _content(content)
    {
    }


    UP<FileNode> ParseConfigurationFile(const Console& console, FileContent& content)
    {
        Lexer lexer(console, content);
        Parser parser(console, lexer, content);

        return parser.ParseFile();
    }

/*
File
    PropertyList endOfInput
 */
    UP<FileNode> Parser::ParseFile()
    {
        auto propList = ParsePropertyList();
        if (propList == nullptr)
            return nullptr;

        auto endOfInput = RequireToken(TokenType::EndOfInput);
        if (endOfInput == nullptr)
            return nullptr;

        return UP<FileNode>(new FileNode(std::move(propList), std::move(endOfInput)));
    }

/*
PropertyList
    Property PropertyList
    Property
 */
    UP<PropertyListNode> Parser::ParsePropertyList()
    {
        std::vector<UP<PropertyNode>> properties;
        do
        {
            if (auto prop = ParseProperty())
                properties.push_back(std::move(prop));
            else
                return nullptr;

        } while (_lexer.PeekType() == TokenType::Word);

        if (properties.size() == 0)
            return nullptr;

        return UP<PropertyListNode>(new PropertyListNode(std::move(properties)));
    }

/*
Property
    PropertyDeclaration PropertyValue endOfLine
    PropertyDeclaration endOfLine PropertyBlock_opt
 */
    UP<PropertyNode> Parser::ParseProperty()
    {
        auto declaration = ParsePropertyDeclaration();
        if (declaration == nullptr)
            return nullptr;

        if (_lexer.PeekType() == TokenType::EndOfLine)
        {
            auto endOfLine = _lexer.Advance();

            if (_lexer.PeekType() != TokenType::Indent)
            {
                // no optional block
                return UP<PropertyNode>(new PropertyNode(std::move(declaration), std::move(endOfLine)));
            }

            auto block = ParsePropertyBlock();
            if (block == nullptr)
                return nullptr;

            return UP<PropertyNode>(new PropertyNode(std::move(declaration), std::move(endOfLine), std::move(block)));
        }

        auto value = ParsePropertyValue();
        if (value == nullptr)
            return nullptr;

        auto endOfLine = RequireToken(TokenType::EndOfLine);
        if (endOfLine == nullptr)
            return nullptr;

        return UP<PropertyNode>(new PropertyNode(std::move(declaration), std::move(value), std::move(endOfLine)));
    }

/*
PropertyDeclaration
    ## property type, followed by an optional name
    word word
    word quotedText
    word
 */
    UP<PropertyDeclarationNode> Parser::ParsePropertyDeclaration()
    {
        auto type = RequireToken(TokenType::Word);
        if (type == nullptr)
            return nullptr;

        auto peekType = _lexer.PeekType();
        if (peekType == TokenType::Word || peekType == TokenType::QuotedText)
        {
            auto name = _lexer.Advance();
            return UP<PropertyDeclarationNode>(new PropertyDeclarationNode(std::move(type), std::move(name)));
        }

        return UP<PropertyDeclarationNode>(new PropertyDeclarationNode(std::move(type)));
    }

/*
PropertyValue
    : quotedText
    : lineText
    > blockText
 */
    UP<PropertyValueNode> Parser::ParsePropertyValue()
    {
        auto peekType = _lexer.PeekType();
        if (peekType == TokenType::Colon)
        {
            auto colon = _lexer.Advance();

            peekType = _lexer.PeekType();
            if (peekType == TokenType::LineText || peekType == TokenType::QuotedText)
            {
                auto text = _lexer.Advance();
                return UP<PropertyValueNode>(new PropertyValueNode(std::move(colon), std::move(text)));
            }

            UnexpectedToken(TokenType::LineText, TokenType::QuotedText);
            return nullptr;
        }

        if (peekType == TokenType::GreaterThan)
        {
            auto greaterThan = _lexer.Advance();

            auto blockText = RequireToken(TokenType::BlockText);
            if (blockText == nullptr)
                return nullptr;

            return UP<PropertyValueNode>(new PropertyValueNode(std::move(greaterThan), std::move(blockText)));
        }

        UnexpectedToken(TokenType::Colon, TokenType::GreaterThan);
        return nullptr;
    }

/*
PropertyBlock
    indent PropertyList outdent
 */
    UP<PropertyBlockNode> Parser::ParsePropertyBlock()
    {
        auto indent = RequireToken(TokenType::Indent);
        if (indent == nullptr)
            return nullptr;

        auto propList = ParsePropertyList();
        if (propList == nullptr)
            return nullptr;

        auto outdent = RequireToken(TokenType::Outdent);
        if (outdent == nullptr)
            return nullptr;

        return UP<PropertyBlockNode>{new PropertyBlockNode(std::move(indent), std::move(propList), std::move(outdent))};
    }

    UP<Token> Parser::RequireToken(TokenType type)
    {
        auto token = _lexer.Consume(type);
        if (token == nullptr)
            UnexpectedToken(type);

        return token;
    }

    void Parser::UnexpectedToken(TokenType expected)
    {
        UnexpectedToken(1, &expected);
    }

    void Parser::UnexpectedToken(TokenType expected0, TokenType expected1)
    {
        TokenType expected[] = {expected0, expected1};
        UnexpectedToken(2, expected);
    }

    void Parser::UnexpectedToken(int expectedCount, TokenType* expectedList)
    {
        auto token = _lexer.Advance();

        if (token->Type() != TokenType::Error)
        {
            _console.Error() << "Error: Unexpected Token " << token->Type() << '\n';
            _console.Error() << FmtPosition(*token);

            if (expectedCount > 0)
            {
                _console.Error() << "    Expected: ";

                for (auto i = 0; i < expectedCount; i++)
                {
                    if (i > 0)
                        _console.Error() << ", ";

                    _console.Error() << expectedList[i];
                }

                _console.Error() << '\n';
            }
        }
    }
}
