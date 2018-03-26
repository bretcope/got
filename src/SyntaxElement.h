#ifndef GOT_SYNTAXELEMENT_H
#define GOT_SYNTAXELEMENT_H

class SyntaxElement
{
public:
    virtual bool IsToken() const = 0;
    inline bool IsNode() const { return !IsToken(); }
};

#endif //GOT_SYNTAXELEMENT_H
