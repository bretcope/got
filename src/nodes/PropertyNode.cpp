#include <cassert>
#include "../Nodes.h"

PropertyNode::PropertyNode(PropertyDeclarationNode* declaration, Token* endOfLine):
        _declaration(declaration),
        _value(nullptr),
        _endOfLine(endOfLine),
        _block(nullptr)
{
    assert(declaration != nullptr);
    assert(endOfLine != nullptr && endOfLine->Type == TokenType::EndOfLine);
}

PropertyNode::PropertyNode(PropertyDeclarationNode* declaration, PropertyValueNode* value, Token* endOfLine):
        _declaration(declaration),
        _value(value),
        _endOfLine(endOfLine),
        _block(nullptr)
{
    assert(declaration != nullptr);
    assert(value != nullptr);
    assert(endOfLine != nullptr && endOfLine->Type == TokenType::EndOfLine);
}

PropertyNode::PropertyNode(PropertyDeclarationNode* declaration, Token* endOfLine, PropertyBlockNode* block):
        _declaration(declaration),
        _value(nullptr),
        _endOfLine(endOfLine),
        _block(block)
{
    assert(declaration != nullptr);
    assert(endOfLine != nullptr && endOfLine->Type == TokenType::EndOfLine);
    assert(block != nullptr);
}

PropertyNode::~PropertyNode()
{
    delete _declaration;
    delete _value;
    delete _endOfLine;
    delete _block;
}

NodeType PropertyNode::Type() const
{
    return NodeType::Property;
}

void PropertyNode::IterateSyntaxElements(std::function<void(const SyntaxElement*)> callback) const
{
    callback(_declaration);

    if (_value != nullptr)
        callback(_value);

    callback(_endOfLine);

    if (_block != nullptr)
        callback(_block);
}

const PropertyDeclarationNode* PropertyNode::Declaration() const
{
    return _declaration;
}

bool PropertyNode::HasValue() const
{
    return _value != nullptr;
}

bool PropertyNode::HasBlock() const
{
    return _block != nullptr;
}

const PropertyValueNode* PropertyNode::Value() const
{
    return _value;
}

const PropertyBlockNode* PropertyNode::Block() const
{
    return _block;
}