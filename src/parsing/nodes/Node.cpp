#include "../Nodes.h"

namespace mot
{
    bool Node::IsToken() const
    {
        return false;
    }

    static int VisitTokensImpl(const Node* node, std::function<void(const Token*)> callback)
    {
        std::vector<const SyntaxElement*> list;
        node->GetSyntaxElements(list);
        auto count = 0;

        for (auto it = list.begin(); it != list.end(); ++it)
        {
            auto element = *it;
            if (element->IsToken())
            {
                callback((const Token*)element);
                count++;
            }
            else if (element->IsNode())
            {
                count += VisitTokensImpl((const Node*)element, callback);
            }
        }

        return count;
    }

    int Node::VisitTokens(std::function<void(const Token*)> callback) const
    {
        return VisitTokensImpl(this, callback);
    }

    static int VisitNodesImpl(const Node* node, std::function<void(const Node* node, int level)> callback, int level)
    {
        std::vector<const SyntaxElement*> list;
        node->GetSyntaxElements(list);
        auto count = 1; // we start at 1 because we know the current node has been sent to the callback already

        for (auto it = list.begin(); it != list.end(); ++it)
        {
            auto element = *it;
            if (element->IsNode())
            {
                auto child = (const Node*)element;
                callback(child, level);
                count += VisitNodesImpl(child, callback, level + 1);
            }
        }

        return count;
    }

    int Node::VisitNodes(std::function<void(const Node* node, int level)> callback) const
    {
        callback(this, 0);
        return VisitNodesImpl(this, callback, 1);
    }

    static const Token* ResolveToken(const SyntaxElement* element, bool left)
    {
        std::vector<const SyntaxElement*> list;
        while (element->IsNode())
        {
            list.clear();
            ((const Node*)element)->GetSyntaxElements(list);
            assert(list.size() > 0);
            element = left ? list.front() : list.back();
        }

        assert(element->IsToken());
        return (const Token*)element;
    }

    FileSpan Node::Position() const
    {
        auto first = ResolveToken(this, true);
        auto last = ResolveToken(this, false);

        return FileSpan(first->Text().Content(), first->Text().Start(), last->Text().End());
    }

    void Node::PrintFileAndPosition(FILE* stream, bool endLine) const
    {
        auto pos = Position();
        pos.Content()->PrintFileAndPosition(stream, pos.Start(), endLine);
    }
}
