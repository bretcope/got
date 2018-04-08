#include <cassert>
#include "../Nodes.h"

namespace mot
{
    FileNode::FileNode(PropertyListNode* propertyList, Token* endOfInput) :
            _propertyList(propertyList),
            _endOfInput(endOfInput)
    {
        assert(propertyList != nullptr);
        assert(endOfInput != nullptr && endOfInput->Type() == TokenType::EndOfInput);
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

    void FileNode::GetSyntaxElements(std::vector<const SyntaxElement*>& list) const
    {
        list.push_back(_propertyList);
        list.push_back(_endOfInput);
    }

    const PropertyListNode* FileNode::PropertyList() const
    {
        return _propertyList;
    }

    const char* FileNode::Filename() const
    {
        return _endOfInput->Filename();
    }
}
