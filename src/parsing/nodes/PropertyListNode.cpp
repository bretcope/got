#include <cassert>
#include "../Nodes.h"

namespace mot
{
    PropertyListNode::PropertyListNode(PropertyNode** properties, int count) :
            _properties(properties),
            _count(count)
    {
        assert(properties != nullptr && count > 0);
    }

    PropertyListNode::~PropertyListNode()
    {
        auto props = _properties;
        auto count = _count;
        for (auto i = 0; i < count; i++)
        {
            delete props[i];
        }

        delete[] props;
    }

    NodeType PropertyListNode::Type() const
    {
        return NodeType::PropertyList;
    }

    void PropertyListNode::GetSyntaxElements(std::vector<const SyntaxElement*>& list) const
    {
        auto props = _properties;
        auto count = _count;

        for (auto i = 0; i < count; i++)
        {
            list.push_back(props[i]);
        }
    }

    const char* PropertyListNode::Filename() const
    {
        return _properties[0]->Filename();
    }

    const PropertyNode* PropertyListNode::GetProperty(int index) const
    {
        return _properties[index];
    }

    int PropertyListNode::Count() const
    {
        return _count;
    }
}
