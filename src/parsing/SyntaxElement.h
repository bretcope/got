#ifndef MOT_SYNTAXELEMENT_H
#define MOT_SYNTAXELEMENT_H

namespace mot
{
    class SyntaxElement
    {
    public:
        virtual bool IsToken() const = 0;
        inline bool IsNode() const { return !IsToken(); }
    };
}

#endif //MOT_SYNTAXELEMENT_H
