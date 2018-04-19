#ifndef MOT_NODES_H
#define MOT_NODES_H

#include <memory>
#include <functional>
#include <cassert>
#include <vector>
#include "Token.h"
#include "SyntaxElement.h"

namespace mot
{
    enum class NodeType
    {
        File,
        PropertyList,
        Property,
        PropertyDeclaration,
        PropertyValue,
        PropertyBlock,
    };

    inline const char* GetNodeTypeName(NodeType type)
    {
        switch (type)
        {
            case NodeType::File:
                return "File";
            case NodeType::PropertyList:
                return "PropertyList";
            case NodeType::Property:
                return "Property";
            case NodeType::PropertyDeclaration:
                return "PropertyDeclaration";
            case NodeType::PropertyValue:
                return "PropertyValue";
            case NodeType::PropertyBlock:
                return "PropertyBlock";
        }

        assert(!"NodeType missing from GetNodeTypeName");
        return "";
    }

    inline std::ostream& operator<<(std::ostream& os, NodeType type)
    {
        os << GetNodeTypeName(type);
        return os;
    }

    class Node : public SyntaxElement
    {
    public:
        virtual NodeType Type() const = 0;

        virtual void GetSyntaxElements(std::vector<const SyntaxElement*>& list) const = 0;

        int VisitTokens(std::function<void(const Token& token)> callback) const;
        int VisitNodes(std::function<void(const Node & node, int level)> callback) const;
        FileSpan Position() const;
    };

    class FileNode;

    class PropertyListNode;

    class PropertyNode;

    class PropertyDeclarationNode;

    class PropertyValueNode;

    class PropertyBlockNode;

    class FileNode final : public Node
    {
    private:
        UP<PropertyListNode> _propertyList;
        UP<Token> _endOfInput;

    public:
        FileNode(UP<PropertyListNode>&& propertyList, UP<Token>&& endOfInput);
        FileNode(const FileNode&) = delete;
        FileNode(FileNode&&) = delete;

        NodeType Type() const override;

        void GetSyntaxElements(std::vector<const SyntaxElement*>& list) const override;

        PropertyListNode const& PropertyList() const;
    };

    class PropertyListNode final : public Node
    {
    private:
        std::vector<mot::UP<PropertyNode>> _properties;

    public:
        explicit PropertyListNode(std::vector<mot::UP<PropertyNode>>&& properties);
        PropertyListNode(const PropertyListNode&) = delete;
        PropertyListNode(PropertyListNode&&) = delete;

        NodeType Type() const override;

        void GetSyntaxElements(std::vector<const SyntaxElement*>& list) const override;

        uint32_t Count() const;
        const PropertyNode& Property(uint32_t index) const;
    };


    class PropertyNode final : public Node
    {
    private:
        UP<PropertyDeclarationNode> _declaration;
        UP<PropertyValueNode> _value;
        UP<Token> _endOfLine;
        UP<PropertyBlockNode> _block;

    public:
        PropertyNode(UP<PropertyDeclarationNode>&& declaration, UP<Token>&& endOfLine);
        PropertyNode(UP<PropertyDeclarationNode>&& declaration, UP<PropertyValueNode>&& value, UP<Token>&& endOfLine);
        PropertyNode(UP<PropertyDeclarationNode>&& declaration, UP<Token>&& endOfLine, UP<PropertyBlockNode>&& block);
        PropertyNode(const PropertyNode&) = delete;

        NodeType Type() const override;

        void GetSyntaxElements(std::vector<const SyntaxElement*>& list) const override;

        const PropertyDeclarationNode& Declaration() const;

        bool HasValue() const;

        bool HasBlock() const;

        const PropertyValueNode* ValueNode() const;

        const PropertyBlockNode* Block() const;
    };

    class PropertyDeclarationNode final : public Node
    {
    private:
        UP<Token> _type;
        UP<Token> _name;

    public:
        explicit PropertyDeclarationNode(UP<Token>&& type);
        PropertyDeclarationNode(UP<Token>&& type, UP<Token>&& name);
        PropertyDeclarationNode(const PropertyDeclarationNode&) = delete;
        PropertyDeclarationNode(PropertyDeclarationNode&&) = delete;

        NodeType Type() const override;

        void GetSyntaxElements(std::vector<const SyntaxElement*>& list) const override;

        MotString PropertyType() const;
        MotString PropertyName() const;
    };

    class PropertyValueNode final : public Node
    {
        UP<Token> _specifier;
        UP<Token> _text;

    public:
        PropertyValueNode(UP<Token>&& specifier, UP<Token>&& text);
        PropertyValueNode(const PropertyValueNode&) = delete;
        PropertyValueNode(PropertyValueNode&&) = delete;

        NodeType Type() const override;

        void GetSyntaxElements(std::vector<const SyntaxElement*>& list) const override;

        MotString Value() const;
    };

    class PropertyBlockNode final : public Node
    {
    private:
        UP<Token> _indent;
        UP<PropertyListNode> _propertyList;
        UP<Token> _outdent;

    public:
        PropertyBlockNode(UP<Token>&& indent, UP<PropertyListNode>&& propertyList, UP<Token>&& outdent);
        PropertyBlockNode(const PropertyBlockNode&) = delete;
        PropertyBlockNode(PropertyBlockNode&&) = delete;

        NodeType Type() const override;

        void GetSyntaxElements(std::vector<const SyntaxElement*>& list) const override;

        const PropertyListNode& PropertyList() const;
    };
}

#endif //MOT_NODES_H
