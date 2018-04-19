#include <cassert>
#include "../Nodes.h"

namespace mot
{
    PropertyValueNode::PropertyValueNode(UP<Token>&& specifier, UP<Token>&& text):
            _specifier{std::move(specifier)},
            _text{std::move(text)}
    {
        assert(_specifier != nullptr);
        assert(_text != nullptr);

        assert(_specifier->Type() == TokenType::Colon || _specifier->Type() == TokenType::GreaterThan);
        assert(_text->Type() == TokenType::LineText || _text->Type() == TokenType::QuotedText || _text->Type() == TokenType::BlockText);

        assert((_specifier->Type() == TokenType::Colon) ^ (_text->Type() != TokenType::LineText && _text->Type() != TokenType::QuotedText));
        assert((_specifier->Type() == TokenType::GreaterThan) ^ (_text->Type() != TokenType::BlockText));
    }

    NodeType PropertyValueNode::Type() const
    {
        return NodeType::PropertyValue;
    }

    void PropertyValueNode::GetSyntaxElements(std::vector<const SyntaxElement*>& list) const
    {
        list.push_back(_specifier.get());
        list.push_back(_text.get());
    }

    MotString PropertyValueNode::Value() const
    {
        return _text->Value();
    }
}