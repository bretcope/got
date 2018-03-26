#include <cassert>
#include "../Nodes.h"

PropertyBlockNode::PropertyBlockNode(Token* indent, PropertyListNode* propertyList, Token* outdent):
        _indent(indent),
        _propertyList(propertyList),
        _outdent(outdent)
{
    assert(indent != nullptr && indent->Type == TokenType::Indent);
    assert(propertyList != nullptr);
    assert(outdent != nullptr && outdent->Type == TokenType::Outdent);
}

PropertyBlockNode::~PropertyBlockNode()
{
    delete _indent;
    delete _propertyList;
    delete _outdent;
}

NodeType PropertyBlockNode::Type() const
{
    return NodeType::PropertyBlock;
}

void PropertyBlockNode::IterateSyntaxElements(std::function<void(const SyntaxElement*)> callback) const
{
    callback(_indent);
    callback(_propertyList);
    callback(_outdent);
}
