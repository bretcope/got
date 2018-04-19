#include <cassert>
#include "../Nodes.h"

namespace mot
{
    FileNode::FileNode(UP<PropertyListNode>&& propertyList, UP<Token>&& endOfInput) :
            _propertyList{std::move(propertyList)},
            _endOfInput{std::move(endOfInput)}
    {
        assert(_propertyList != nullptr);
        assert(_endOfInput != nullptr && _endOfInput->Type() == TokenType::EndOfInput);
    }

    NodeType FileNode::Type() const
    {
        return NodeType::File;
    }

    void FileNode::GetSyntaxElements(std::vector<const SyntaxElement*>& list) const
    {
        list.push_back(_propertyList.get());
        list.push_back(_endOfInput.get());
    }

    const PropertyListNode& FileNode::PropertyList() const
    {
        return *_propertyList;
    }
}
