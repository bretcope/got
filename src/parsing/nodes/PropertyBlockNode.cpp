#include <cassert>
#include "../Nodes.h"

namespace mot
{
    PropertyBlockNode::PropertyBlockNode(Token* indent, PropertyListNode* propertyList, Token* outdent) :
            _indent(indent),
            _propertyList(propertyList),
            _outdent(outdent)
    {
        assert(indent != nullptr && indent->Type() == TokenType::Indent);
        assert(propertyList != nullptr);
        assert(outdent != nullptr && outdent->Type() == TokenType::Outdent);
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

    void PropertyBlockNode::GetSyntaxElements(std::vector<const SyntaxElement*>& list) const
    {
        list.push_back(_indent);
        list.push_back(_propertyList);
        list.push_back(_outdent);
    }

    const char* PropertyBlockNode::Filename() const
    {
        return _indent->Filename();
    }
}
