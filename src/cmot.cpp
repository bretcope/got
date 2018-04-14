#include <iostream>
#include <fstream>
#include <locale>
#include "io/FileReader.h"
#include "parsing/Lexer.h"
#include "parsing/Parser.h"
#include "text/Utf8.h"
#include "ir/FileIR.h"

using namespace mot;

void DebugLexer(const Console& console, FileContent* content)
{
    Lexer lexer(console, content);

    while (auto token = lexer.Advance())
    {
        token->DebugPrint(console.Out(), true, true);
        delete token;
    }
}

void DebugParser(const Console& console, FileContent* content)
{
    FileNode* tree;
    if (ParseConfigurationFile(console, content, &tree))
    {
        auto callback = [&console] (const Node* node, int level) -> void
        {
            for (auto i = 0; i < level; i++)
            {
                console.Out() << "  ";
            }

            console.Out() << node->Type() << '\n';
        };

        tree->VisitNodes(callback);
        delete tree;
    }
}

void DebugIR(const Console& console, FileContent* content)
{
    FileNode* tree;
    if (ParseConfigurationFile(console, content, &tree))
    {
        FileIR ir(console, tree, false);

        if (ir.IsValid())
        {
            auto prefixes = ir.Prefixes();
            console.Out() << "Found " << prefixes.size() << " prefixes.\n";
            for (auto kvp : prefixes)
            {
                console.Out() << "prefix: " << kvp.first << "\n";
            }
        }
    }
}

int main(int argc, char** argv)
{
    // todo: better console type detection (support arguments)
#if WIN32
   auto consoleType = ConsoleType::Windows;
#else
   auto consoleType = ConsoleType::Unix;
#endif

   Console console(consoleType, std::cout, std::cerr);

//    MotString one("one");
//    MotString ONE("ONE");
//    MotString OnE("OnE");
//    MotString two("two");
//
//    MotString RU("heyП");
//    MotString ru("heyп");
//
//    std::cout << one.HashCode() << "\n";
//    std::cout << ONE.HashCode() << "\n";
//    std::cout << OnE.HashCode() << "\n";
//    std::cout << two.HashCode() << "\n";
//    std::cout << RU.HashCode() << "\n";
//    std::cout << ru.HashCode() << "\n";
//
//    std::cout << MotString::Compare(&one, &ONE) << "\n";
//    std::cout << MotString::CompareCaseInsensitive(&one, &ONE) << "\n";
//    std::cout << MotString::CompareCaseInsensitive(&one, &OnE) << "\n";
//    std::cout << MotString::CompareCaseInsensitive(&one, &two) << "\n";
//    std::cout << MotString::Compare(&RU, &ru) << "\n";
//    std::cout << MotString::CompareCaseInsensitive(&RU, &ru) << "\n";
//
//    std::cout << one << "\n";
//    std::cout << ru << "\n";
//
//    return 0;

    const char* filename = "sample_profile";

    const uint32_t MAX_SIZE = 10 * 1024 * 1024;
    auto content = LoadFile(console, filename, MAX_SIZE);
    if (content == nullptr)
        return 1;

//        std::cout.write(content, size);
//        DebugLexer(content);
//        DebugParser(console, content);
    DebugIR(console, content.get());

    // execute command https://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c-using-posix

    return 0;
}