#include "../Nodes.h"

bool Node::IsToken() const
{
    return false;
}

int Node::VisitTokens(std::function<void(const Token*)> callback) const
{
    auto count = 0;
    std::function<void (const SyntaxElement*)> visitCallack = [&] (const SyntaxElement* element) -> void
    {
        if (element->IsToken())
        {
            callback((Token*)element);
            count++;
        }
        else if (element->IsNode())
        {
            ((Node*)element)->IterateSyntaxElements(visitCallack);
        }
    };

    IterateSyntaxElements(visitCallack);
    return count;
}

int Node::VisitNodes(std::function<void(const Node*)> callback) const
{
    callback(this);
    auto count = 1;

    std::function<void (const SyntaxElement*)> visitCallback = [&] (const SyntaxElement* element) -> void
    {
        if (element->IsNode())
        {
            auto node = (Node*)element;
            callback(node);
            count++;
            node->IterateSyntaxElements(visitCallback);
        }
    };

    IterateSyntaxElements(visitCallback);
    return count;
}
