#include "Console.h"

namespace mot
{
    Console::Console(ConsoleType type, std::ostream& out, std::ostream& err):
            _out(out),
            _err(err),
            _type(type)
    {
    }

    ConsoleType Console::Type() const
    {
        return _type;
    }

    std::ostream& Console::Out() const
    {
        return _out;
    }

    std::ostream& Console::Error() const
    {
        return _err;
    }
}
