#ifndef MOT_CONSOLE_H
#define MOT_CONSOLE_H


#include <ostream>
#include "ConsoleType.h"

namespace mot
{
    class Console
    {
    private:
        std::ostream& _out;
        std::ostream& _err;
        ConsoleType _type;

    public:
        Console(ConsoleType type, std::ostream& out, std::ostream& err);

        ConsoleType Type() const;
        std::ostream& Out() const;
        std::ostream& Error() const;

        const char* EndLine() const;
        // todo: std::ostream& Log(area) const;
    };
}


#endif //MOT_CONSOLE_H
