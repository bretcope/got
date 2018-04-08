#include <cassert>
#include "../Nodes.h"

namespace mot
{
    PropertyNode::PropertyNode(PropertyDeclarationNode* declaration, Token* endOfLine) :
            _declaration(declaration),
            _value(nullptr),
            _endOfLine(endOfLine),
            _block(nullptr)
    {
        assert(declaration != nullptr);
        assert(endOfLine != nullptr && endOfLine->Type() == TokenType::EndOfLine);
    }

    PropertyNode::PropertyNode(PropertyDeclarationNode* declaration, PropertyValueNode* value, Token* endOfLine) :
            _declaration(declaration),
            _value(value),
            _endOfLine(endOfLine),
            _block(nullptr)
    {
        assert(declaration != nullptr);
        assert(value != nullptr);
        assert(endOfLine != nullptr && endOfLine->Type() == TokenType::EndOfLine);
    }

    PropertyNode::PropertyNode(PropertyDeclarationNode* declaration, Token* endOfLine, PropertyBlockNode* block) :
            _declaration(declaration),
            _value(nullptr),
            _endOfLine(endOfLine),
            _block(block)
    {
        assert(declaration != nullptr);
        assert(endOfLine != nullptr && endOfLine->Type() == TokenType::EndOfLine);
        assert(block != nullptr);
    }

    PropertyNode::~PropertyNode()
    {
        delete _declaration;
        delete _value;
        delete _endOfLine;
        delete _block;
    }

    NodeType PropertyNode::Type() const
    {
        return NodeType::Property;
    }

    void PropertyNode::GetSyntaxElements(std::vector<const SyntaxElement*>& list) const
    {
        list.push_back(_declaration);

        if (_value != nullptr)
            list.push_back(_value);

        list.push_back(_endOfLine);

        if (_block != nullptr)
            list.push_back(_block);
    }

    const char* PropertyNode::Filename() const
    {
        return _endOfLine->Filename();
    }

    const PropertyDeclarationNode* PropertyNode::Declaration() const
    {
        return _declaration;
    }

    bool PropertyNode::HasValue() const
    {
        return _value != nullptr;
    }

    bool PropertyNode::HasBlock() const
    {
        return _block != nullptr;
    }

    const PropertyValueNode* PropertyNode::Value() const
    {
        return _value;
    }

    const PropertyBlockNode* PropertyNode::Block() const
    {
        return _block;
    }
}
