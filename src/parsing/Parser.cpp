#include <cstring>
#include <algorithm>
#include "Parser.h"
#include "Lexer.h"

class Parser
{
    Lexer* _lexer;
    FileContent* _content;
    FILE* _errStream;

public:
    Parser(Lexer* lexer, FileContent* content, FILE* errStream);
    ~Parser();

    bool ParseFile(FileNode** out_node);
    bool ParsePropertyList(PropertyListNode** out_node);
    bool ParseProperty(PropertyNode** out_node);
    bool ParsePropertyDeclaration(PropertyDeclarationNode** out_node);
    bool ParsePropertyValue(PropertyValueNode** out_node);
    bool ParsePropertyBlock(PropertyBlockNode** out_node);

    bool RequireToken(TokenType type, Token** out_token);

    void UnexpectedToken(TokenType expected);
    void UnexpectedToken(TokenType expected0, TokenType expected1);
    void UnexpectedToken(int expectedCount, TokenType* expectedList);
};

Parser::Parser(Lexer* lexer, FileContent* content, FILE* errStream):
        _lexer(lexer),
        _content(content),
        _errStream(errStream)
{
}

Parser::~Parser() = default;


bool ParseConfigurationFile(FileContent* content, FILE* errStream, FileNode** out_tree)
{
    Lexer lexer(content, errStream);
    Parser parser(&lexer, content, errStream);

    return parser.ParseFile(out_tree);
}

/*
File
    PropertyList endOfInput
 */
bool Parser::ParseFile(FileNode** out_node)
{
    PropertyListNode* propList = nullptr;
    Token* endOfInput = nullptr;
    if (ParsePropertyList(&propList) && RequireToken(TokenType::EndOfInput, &endOfInput))
    {
        *out_node = new FileNode(propList, endOfInput);
        return true;
    }

    delete propList;
    delete endOfInput;
    return false;
}

/*
PropertyList
    Property PropertyList
    Property
 */
bool Parser::ParsePropertyList(PropertyListNode** out_node)
{
    auto count = 0;
    auto capacity = 0;
    auto success = false;
    PropertyNode** properties = nullptr; // this should probably be std::vector<PropertyNode*> or some other wrapper, but... ¯\_(ツ)_/¯

    do
    {
        PropertyNode* prop;
        if (ParseProperty(&prop))
        {
            success = true;
            if (count >= capacity)
            {
                auto newCapacity = std::max(8, capacity * 2);
                auto newProps = new PropertyNode*[newCapacity];

                if (properties != nullptr)
                {
                    memcpy(newProps, properties, sizeof(PropertyNode*) * count);
                    delete properties;
                }

                capacity = newCapacity;
                properties = newProps;
            }

            properties[count] = prop;
            count++;
        }
        else
        {
            success = false;
            break;
        }

    } while (_lexer->PeekType() == TokenType::Word);

    if (success)
    {
        *out_node = new PropertyListNode(properties, count);
        return true;
    }

    for (auto i = 0; i < count; i++)
    {
        delete properties[i];
    }

    delete properties;
    *out_node = nullptr;
    return false;
}

/*
Property
    PropertyDeclaration PropertyValue endOfLine
    PropertyDeclaration endOfLine PropertyBlock_opt
 */
bool Parser::ParseProperty(PropertyNode** out_node)
{
    PropertyDeclarationNode* declaration;
    if (!ParsePropertyDeclaration(&declaration))
        return false;

    if (_lexer->PeekType() == TokenType::EndOfLine)
    {
        Token* endOfLine = _lexer->Advance();

        if (_lexer->PeekType() != TokenType::Indent)
        {
            // no optional block
            *out_node = new PropertyNode(declaration, endOfLine);
            return true;
        }

        PropertyBlockNode* block;
        if (ParsePropertyBlock(&block))
        {
            *out_node = new PropertyNode(declaration, endOfLine, block);
            return true;
        }

        delete endOfLine;
        delete block;
    }
    else
    {
        PropertyValueNode* value = nullptr;
        Token* endOfLine = nullptr;
        if (ParsePropertyValue(&value) && RequireToken(TokenType::EndOfLine, &endOfLine))
        {
            *out_node = new PropertyNode(declaration, value, endOfLine);
            return true;
        }

        delete value;
        delete endOfLine;
    }

    delete declaration;
    *out_node = nullptr;
    return false;
}

/*
PropertyDeclaration
    ## property type, followed by an optional name
    word word
    word quotedText
    word
 */
bool Parser::ParsePropertyDeclaration(PropertyDeclarationNode** out_node)
{
    Token* type;
    if (!RequireToken(TokenType::Word, &type))
    {
        *out_node = nullptr;
        return false;
    }

    auto peekType = _lexer->PeekType();
    if (peekType == TokenType::Word || peekType == TokenType::QuotedText)
    {
        auto name = _lexer->Advance();
        *out_node = new PropertyDeclarationNode(type, name);
    }
    else
    {
        *out_node = new PropertyDeclarationNode(type);
    }

    return true;
}

/*
PropertyValue
    : quotedText
    : lineText
    > blockText
 */
bool Parser::ParsePropertyValue(PropertyValueNode** out_node)
{
    auto peekType = _lexer->PeekType();
    if (peekType == TokenType::Colon)
    {
        auto colon = _lexer->Advance();

        peekType = _lexer->PeekType();
        if (peekType == TokenType::LineText || peekType == TokenType::QuotedText)
        {
            auto text = _lexer->Advance();
            *out_node = new PropertyValueNode(colon, text);
            return true;
        }

        UnexpectedToken(TokenType::LineText, TokenType::QuotedText);
        delete colon;
    }
    else if (peekType == TokenType::GreaterThan)
    {
        auto greaterThan = _lexer->Advance();

        Token* blockText;
        if (RequireToken(TokenType::BlockText, &blockText))
        {
            *out_node = new PropertyValueNode(greaterThan, blockText);
            return true;
        }

        delete greaterThan;
    }
    else
    {
        UnexpectedToken(TokenType::Colon, TokenType::GreaterThan);
    }

    *out_node = nullptr;
    return false;
}

/*
PropertyBlock
    indent PropertyList outdent
 */
bool Parser::ParsePropertyBlock(PropertyBlockNode** out_node)
{
    Token* indent = nullptr;
    PropertyListNode* propList = nullptr;
    Token* outdent = nullptr;

    if (RequireToken(TokenType::Indent, &indent) && ParsePropertyList(&propList) && RequireToken(TokenType::Outdent, &outdent))
    {
        *out_node = new PropertyBlockNode(indent, propList, outdent);
        return true;
    }

    delete indent;
    delete propList;
    delete outdent;
    *out_node = nullptr;
    return false;
}

bool Parser::RequireToken(TokenType type, Token** out_token)
{
    if (_lexer->Consume(type, out_token))
        return true;

    UnexpectedToken(type);
    *out_token = nullptr;
    return false;
}

void Parser::UnexpectedToken(TokenType expected)
{
    UnexpectedToken(1, &expected);
}

void Parser::UnexpectedToken(TokenType expected0, TokenType expected1)
{

    TokenType expected[] = { expected0, expected1 };
    UnexpectedToken(2, expected);
}

void Parser::UnexpectedToken(int expectedCount, TokenType* expectedList)
{
    // todo: this error message could use a lot of improvement - like at least giving line numbers

    auto token = _lexer->Advance();

    if (token == nullptr)
    {
        fprintf(_errStream, "Parser Error: Lexer returned an null token.\n    File: \"%s\"", _content->Filename());
    }
    else
    {
        fprintf(_errStream, "Error: Cannot parse configuration file\n\n");
        fprintf(_errStream, "    Unexpected Token \"%s\"\n", GetTokenTypeName(token->Type()));
        fprintf(_errStream, "      at \"%s\" %u\n", _content->Filename(), token->Text().Start());
    }

    if (expectedCount > 0)
    {
        fprintf(_errStream, "\n    Expected: ");

        for (auto i = 0; i < expectedCount; i++)
        {
            if (i > 0)
                fwrite(", ", 1, 2, _errStream);

            fprintf(_errStream, "%s", GetTokenTypeName(expectedList[i]));
        }

        fwrite("\n", 1, 1, _errStream);
    }
}
