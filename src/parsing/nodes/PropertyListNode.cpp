#include <cassert>
#include "../Nodes.h"

namespace mot
{
    PropertyListNode::PropertyListNode(std::vector<UP<PropertyNode>>&& properties) :
            _properties{std::move(properties)}
    {
        assert(_properties.size() > 0);
    }

    NodeType PropertyListNode::Type() const
    {
        return NodeType::PropertyList;
    }

    void PropertyListNode::GetSyntaxElements(std::vector<const SyntaxElement*>& list) const
    {
        for (auto it = _properties.begin(); it != _properties.end(); ++it)
        {
            list.push_back((*it).get());
        }
    }

    uint32_t PropertyListNode::Count() const
    {
        return (uint32_t)_properties.size();
    }

    const PropertyNode& PropertyListNode::Property(uint32_t index) const
    {
        return *_properties[index];
    }
}
