#include <cassert>
#include "../Nodes.h"

namespace mot
{
    PropertyDeclarationNode::PropertyDeclarationNode(UP<Token>&& type) :
            _type{std::move(type)},
            _name{nullptr}
    {
        assert(_type != nullptr && _type->Type() == TokenType::Word);
    }

    PropertyDeclarationNode::PropertyDeclarationNode(UP<Token>&& type, UP<Token>&& name) :
            _type{std::move(type)},
            _name{std::move(name)}
    {
        assert(_type != nullptr && _type->Type() == TokenType::Word);
        assert(_name != nullptr && (_name->Type() == TokenType::Word || _name->Type() == TokenType::QuotedText));
    }

    NodeType PropertyDeclarationNode::Type() const
    {
        return NodeType::PropertyDeclaration;
    }

    void PropertyDeclarationNode::GetSyntaxElements(std::vector<const SyntaxElement*>& list) const
    {
        list.push_back(_type.get());

        if (_name != nullptr)
            list.push_back(_name.get());
    }

    MotString PropertyDeclarationNode::PropertyType() const
    {
        return _type->Value();
    }

    MotString PropertyDeclarationNode::PropertyName() const
    {
        return _name == nullptr ? MotString() : _name->Value();
    }
}
