#include <cassert>
#include "../Nodes.h"

namespace mot
{
    PropertyNode::PropertyNode(UP<PropertyDeclarationNode>&& declaration, UP<Token>&& endOfLine) :
            _declaration{std::move(declaration)},
            _value{nullptr},
            _endOfLine{std::move(endOfLine)},
            _block{nullptr}
    {
        assert(_declaration != nullptr);
        assert(_endOfLine != nullptr && _endOfLine->Type() == TokenType::EndOfLine);
    }

    PropertyNode::PropertyNode(UP<PropertyDeclarationNode>&& declaration, UP<PropertyValueNode>&& value, UP<Token>&& endOfLine) :
            _declaration{std::move(declaration)},
            _value{std::move(value)},
            _endOfLine{std::move(endOfLine)},
            _block{nullptr}
    {
        assert(_declaration != nullptr);
        assert(_value != nullptr);
        assert(_endOfLine != nullptr && _endOfLine->Type() == TokenType::EndOfLine);
    }

    PropertyNode::PropertyNode(UP<PropertyDeclarationNode>&& declaration, UP<Token>&& endOfLine, UP<PropertyBlockNode>&& block) :
            _declaration{std::move(declaration)},
            _value{nullptr},
            _endOfLine{std::move(endOfLine)},
            _block{std::move(block)}
    {
        assert(_declaration != nullptr);
        assert(_endOfLine != nullptr && _endOfLine->Type() == TokenType::EndOfLine);
        assert(_block != nullptr);
    }

    NodeType PropertyNode::Type() const
    {
        return NodeType::Property;
    }

    void PropertyNode::GetSyntaxElements(std::vector<const SyntaxElement*>& list) const
    {
        list.push_back(_declaration.get());

        if (_value != nullptr)
            list.push_back(_value.get());

        list.push_back(_endOfLine.get());

        if (_block != nullptr)
            list.push_back(_block.get());
    }

    const PropertyDeclarationNode& PropertyNode::Declaration() const
    {
        return *_declaration;
    }

    bool PropertyNode::HasValue() const
    {
        return _value != nullptr;
    }

    bool PropertyNode::HasBlock() const
    {
        return _block != nullptr;
    }

    const PropertyValueNode* PropertyNode::ValueNode() const
    {
        return _value.get();
    }

    const PropertyBlockNode* PropertyNode::Block() const
    {
        return _block.get();
    }
}
