#include <cassert>
#include "../Nodes.h"

namespace mot
{
    PropertyValueNode::PropertyValueNode(Token* specifier, Token* text) :
            _specifier(specifier),
            _text(text)
    {
        assert(specifier != nullptr);
        assert(text != nullptr);

        assert(specifier->Type() == TokenType::Colon || specifier->Type() == TokenType::GreaterThan);
        assert(text->Type() == TokenType::LineText || text->Type() == TokenType::QuotedText || text->Type() == TokenType::BlockText);

        assert((specifier->Type() == TokenType::Colon) ^ (text->Type() != TokenType::LineText && text->Type() != TokenType::QuotedText));
        assert((specifier->Type() == TokenType::GreaterThan) ^ (text->Type() != TokenType::BlockText));
    }

    PropertyValueNode::~PropertyValueNode()
    {
        delete _specifier;
        delete _text;
    }

    NodeType PropertyValueNode::Type() const
    {
        return NodeType::PropertyValue;
    }

    void PropertyValueNode::GetSyntaxElements(std::vector<const SyntaxElement*>& list) const
    {
        list.push_back(_specifier);
        list.push_back(_text);
    }

    const char* PropertyValueNode::Filename() const
    {
        return _specifier->Filename();
    }

    const MotString* PropertyValueNode::Value() const
    {
        return _text->Value();
    }
}