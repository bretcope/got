#include <cassert>
#include "../Nodes.h"

PropertyValueNode::PropertyValueNode(Token* specifier, Token* text):
        _specifier(specifier),
        _text(text)
{
    assert(specifier != nullptr);
    assert(text != nullptr);

    assert(specifier->Type() == TokenType::Colon || specifier->Type() == TokenType::GreaterThan);
    assert(text->Type() == TokenType::LineText || text->Type() == TokenType::QuotedText || text->Type() == TokenType::BlockText);

    assert((specifier->Type() == TokenType::Colon) ^ (text->Type() != TokenType::LineText && text->Type() != TokenType::QuotedText));
    assert((specifier->Type() == TokenType::GreaterThan) ^ (text->Type() != TokenType::BlockText));
}

PropertyValueNode::~PropertyValueNode()
{
    delete _specifier;
    delete _text;
}

NodeType PropertyValueNode::Type() const
{
    return NodeType::PropertyValue;
}

void PropertyValueNode::IterateSyntaxElements(std::function<void(const SyntaxElement*)> callback) const
{
    callback(_specifier);
    callback(_text);
}