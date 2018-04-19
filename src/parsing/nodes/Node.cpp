#include "../Nodes.h"

namespace mot
{
    static int VisitTokensImpl(const Node& node, std::function<void(const Token& token)> callback)
    {
        std::vector<const SyntaxElement*> list;
        node.GetSyntaxElements(list);
        auto count = 0;

        for (auto it = list.begin(); it != list.end(); ++it)
        {
            auto element = *it;
            if (auto token = dynamic_cast<const Token*>(element))
            {
                callback(*token);
                count++;
            }
            else if (auto childNode = dynamic_cast<const Node*>(element))
            {
                count += VisitTokensImpl(*childNode, callback);
            }
        }

        return count;
    }

    int Node::VisitTokens(std::function<void(const Token& token)> callback) const
    {
        return VisitTokensImpl(*this, callback);
    }

    static int VisitNodesImpl(const Node& node, std::function<void(const Node& node, int level)> callback, int level)
    {
        std::vector<const SyntaxElement*> list;
        node.GetSyntaxElements(list);
        auto count = 1; // we start at 1 because we know the current node has been sent to the callback already

        for (auto it = list.begin(); it != list.end(); ++it)
        {
            const SyntaxElement* element = *it;
            if (auto child = dynamic_cast<const Node*>(element))
            {
                callback(*child, level);
                count += VisitNodesImpl(*child, callback, level + 1);
            }
        }

        return count;
    }

    int Node::VisitNodes(std::function<void(const Node& node, int level)> callback) const
    {
        callback(*this, 0);
        return VisitNodesImpl(*this, callback, 1);
    }

    static const Token& ResolveToken(const SyntaxElement* element, bool left)
    {
        std::vector<const SyntaxElement*> list;
        while (auto node = dynamic_cast<const Node*>(element))
        {
            list.clear();
            node->GetSyntaxElements(list);
            assert(list.size() > 0);
            element = left ? list.front() : list.back();
        }

        return dynamic_cast<const Token&>(*element);
    }

    FileSpan Node::Position() const
    {
        auto first = ResolveToken(this, true);
        auto last = ResolveToken(this, false);

        return FileSpan(first.Text().Content(), first.Text().Start(), last.Text().End());
    }
}
