#include <cassert>
#include "../Nodes.h"

PropertyValueNode::PropertyValueNode(Token* blockText):
        _colon(nullptr),
        _text(blockText)
{
    assert(blockText != nullptr && blockText->Type == TokenType::BlockText);
}

PropertyValueNode::PropertyValueNode(Token* colon, Token* text):
        _colon(colon),
        _text(text)
{
    assert(colon != nullptr && colon->Type == TokenType::Colon);
    assert(text != nullptr && (text->Type == TokenType::LineText || text->Type == TokenType::QuotedText));
}

PropertyValueNode::~PropertyValueNode()
{
    delete _colon;
    delete _text;
}

NodeType PropertyValueNode::Type() const
{
    return NodeType::PropertyValue;
}

void PropertyValueNode::IterateSyntaxElements(std::function<void(const SyntaxElement*)> callback) const
{
    if (_colon != nullptr)
        callback(_colon);

    callback(_text);
}