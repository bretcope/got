#include <cassert>
#include "../Nodes.h"

namespace mot
{
    PropertyBlockNode::PropertyBlockNode(UP<Token>&& indent, UP<PropertyListNode>&& propertyList, UP<Token>&& outdent) :
            _indent{std::move(indent)},
            _propertyList{std::move(propertyList)},
            _outdent{std::move(outdent)}
    {
        assert(_indent != nullptr && _indent->Type() == TokenType::Indent);
        assert(_propertyList != nullptr);
        assert(_outdent != nullptr && _outdent->Type() == TokenType::Outdent);
    }

    NodeType PropertyBlockNode::Type() const
    {
        return NodeType::PropertyBlock;
    }

    void PropertyBlockNode::GetSyntaxElements(std::vector<const SyntaxElement*>& list) const
    {
        list.push_back(_indent.get());
        list.push_back(_propertyList.get());
        list.push_back(_outdent.get());
    }

    const PropertyListNode& PropertyBlockNode::PropertyList() const
    {
        return *_propertyList;
    }
}
