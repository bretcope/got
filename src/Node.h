#ifndef GOT_NODE_H
#define GOT_NODE_H

#include <memory>
#include "Token.h"

//enum class NodeType
//{
//    Start,
//    PropertyList,
//    Property,
//    PropertyDeclaration,
//    PropertyName,
//    Value,
//    PropertyBlock,
//};

//class Node
//{
//};

class StartNode;
class PropertyListNode;
class PropertyNode;
class PropertyDeclarationNode;
class PropertyNameNode;
class ValueNode;
class PropertyBlockNode;

class StartNode
{
public:

    PropertyListNode* PropertyList;

private:
    Token* _endOfInput;

//    union
//    {
//        void*
//    };

};

class PropertyListNode
{
    //
};

class PropertyNode
{
    //
};

class PropertyDeclarationNode
{
    //
};

class PropertyNameNode
{
    //
};

class ValueNode
{
    //
};

class PropertyBlockNode
{
    //
};



#endif //GOT_NODE_H
