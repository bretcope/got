#ifndef MOT_NODES_H
#define MOT_NODES_H

#include <memory>
#include <functional>
#include <cassert>
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

    class Node : public SyntaxElement
    {
    public:
        bool IsToken() const final;

        virtual NodeType Type() const = 0;

        virtual void IterateSyntaxElements(std::function<void(const SyntaxElement*)> callback) const = 0;

        int VisitTokens(std::function<void(const Token*)> callback) const;

        int VisitNodes(std::function<void(const Node* node, int level)> callback) const;
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
        PropertyListNode* _propertyList;
        Token* _endOfInput;

    public:
        FileNode(PropertyListNode* propertyList, Token* endOfInput);
        FileNode(const FileNode&) = delete;
        FileNode(FileNode&&) = delete;
        ~FileNode();

        NodeType Type() const override;

        void IterateSyntaxElements(std::function<void(const SyntaxElement*)> callback) const override;

        const PropertyListNode* PropertyList() const;
    };

    class PropertyListNode final : public Node
    {
    private:
        PropertyNode** _properties;
        int _count;

    public:
        PropertyListNode(PropertyNode** properties, int count);
        PropertyListNode(const PropertyListNode&) = delete;
        PropertyListNode(PropertyListNode&&) = delete;
        ~PropertyListNode();

        NodeType Type() const override;

        void IterateSyntaxElements(std::function<void(const SyntaxElement*)> callback) const override;

        const PropertyNode* GetProperty(int index) const;

        int Count() const;
    };


    class PropertyNode final : public Node
    {
    private:
        PropertyDeclarationNode* _declaration;
        PropertyValueNode* _value;
        Token* _endOfLine;
        PropertyBlockNode* _block;

    public:
        PropertyNode(PropertyDeclarationNode* declaration, Token* endOfLine);
        PropertyNode(PropertyDeclarationNode* declaration, PropertyValueNode* value, Token* endOfLine);
        PropertyNode(PropertyDeclarationNode* declaration, Token* endOfLine, PropertyBlockNode* block);
        PropertyNode(const PropertyNode&) = delete;
        PropertyNode(PropertyNode&&) = delete;
        ~PropertyNode();

        NodeType Type() const override;

        void IterateSyntaxElements(std::function<void(const SyntaxElement*)> callback) const override;

        const PropertyDeclarationNode* Declaration() const;

        bool HasValue() const;

        bool HasBlock() const;

        const PropertyValueNode* Value() const;

        const PropertyBlockNode* Block() const;
    };

    class PropertyDeclarationNode final : public Node
    {
    private:
        Token* _type;
        Token* _name;

    public:
        explicit PropertyDeclarationNode(Token* type);
        PropertyDeclarationNode(Token* type, Token* name);
        PropertyDeclarationNode(const PropertyDeclarationNode&) = delete;
        PropertyDeclarationNode(PropertyDeclarationNode&&) = delete;
        ~PropertyDeclarationNode();

        NodeType Type() const override;

        void IterateSyntaxElements(std::function<void(const SyntaxElement*)> callback) const override;

        const MotString* PropertyType() const;
        const MotString* PropertyName() const;
    };

    class PropertyValueNode final : public Node
    {
        Token* _specifier;
        Token* _text;

    public:
        PropertyValueNode(Token* specifier, Token* text);
        PropertyValueNode(const PropertyValueNode&) = delete;
        PropertyValueNode(PropertyValueNode&&) = delete;
        ~PropertyValueNode();

        NodeType Type() const override;

        void IterateSyntaxElements(std::function<void(const SyntaxElement*)> callback) const override;

        // todo: method to evaluate (parse) the actual value out of the _text token.
        // todo: const char* Value() const;
    };

    class PropertyBlockNode final : public Node
    {
    private:
        Token* _indent;
        PropertyListNode* _propertyList;
        Token* _outdent;

    public:
        PropertyBlockNode(Token* indent, PropertyListNode* propertyList, Token* outdent);
        PropertyBlockNode(const PropertyBlockNode&) = delete;
        PropertyBlockNode(PropertyBlockNode&&) = delete;
        ~PropertyBlockNode();

        NodeType Type() const override;

        void IterateSyntaxElements(std::function<void(const SyntaxElement*)> callback) const override;
    };
}

#endif //MOT_NODES_H
