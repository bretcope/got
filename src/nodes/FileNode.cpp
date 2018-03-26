#include <cassert>
#include "../Nodes.h"

FileNode::FileNode(PropertyListNode* propertyList, Token* endOfInput):
        _propertyList(propertyList),
        _endOfInput(endOfInput)
{
    assert(propertyList != nullptr);
    assert(endOfInput != nullptr && endOfInput->Type == TokenType::EndOfInput);
}

FileNode::~FileNode()
{
    delete _propertyList;
    delete _endOfInput;
}

NodeType FileNode::Type() const
{
    return NodeType::File;
}

void FileNode::IterateSyntaxElements(std::function<void(const SyntaxElement*)> callback) const
{
    callback(_propertyList);
    callback(_endOfInput);
}

const PropertyListNode* FileNode::PropertyList() const
{
    return _propertyList;
}
