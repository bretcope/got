#include <cassert>
#include "../Nodes.h"

PropertyDeclarationNode::PropertyDeclarationNode(Token* type):
        _type(type),
        _name(nullptr)
{
    assert(type != nullptr && type->Type() == TokenType::Word);
}

PropertyDeclarationNode::PropertyDeclarationNode(Token* type, Token* name):
        _type(type),
        _name(name)
{
    assert(type != nullptr && type->Type() == TokenType::Word);
    assert(name != nullptr && (name->Type() == TokenType::Word || name->Type() == TokenType::QuotedText));
}

PropertyDeclarationNode::~PropertyDeclarationNode()
{
    delete _type;
    delete _name;
}

NodeType PropertyDeclarationNode::Type() const
{
    return NodeType::PropertyDeclaration;
}

void PropertyDeclarationNode::IterateSyntaxElements(std::function<void(const SyntaxElement*)> callback) const
{
    callback(_type);

    if (_name != nullptr)
        callback(_name);
}
