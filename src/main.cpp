#include <iostream>
#include <fstream>
#include <locale>
#include "io/FileReader.h"
#include "parsing/Lexer.h"
#include "parsing/Parser.h"
#include "io/Utf8.h"

void DebugLexer(FileContent* content)
{
    auto lexer = Lexer(content, stderr);

    while (auto token = lexer.Advance())
    {
        token->DebugPrint(stdout, true, true);
        delete token;
    }
}

void DebugParser(FileContent* content)
{
    FileNode* tree;
    if (ParseConfigurationFile(content, stderr, &tree))
    {
        auto callback = [] (const Node* node, int level) -> void
        {
            for (auto i = 0; i < level; i++)
            {
                fwrite("  ", 1, 2, stdout);
            }

            puts(GetNodeTypeName(node->Type()));
        };

        tree->VisitNodes(callback);
        delete tree;
    }
}

int main(int argc, char** argv)
{
#if WIN32
   const char* os = "Windows";
#elif UNIX
    const char* os = "Unix";
#else
    const char* os = "Unknown";
#endif

    auto one = MotString::NewFromConstant("one");
    auto ONE = MotString::NewFromConstant("ONE");
    auto OnE = MotString::NewFromConstant("OnE");
    auto two = MotString::NewFromConstant("two");

    auto RU = MotString::NewFromConstant("heyП");
    auto ru = MotString::NewFromConstant("heyп");

    std::cout << one->HashCode() << "\n";
    std::cout << ONE->HashCode() << "\n";
    std::cout << OnE->HashCode() << "\n";
    std::cout << two->HashCode() << "\n";
    std::cout << RU->HashCode() << "\n";
    std::cout << ru->HashCode() << "\n";

    delete one, ONE, OnE, two, RU, ru;

    return 0;

    const char* filename = "sample_profile";

    FileContent* content;
    const uint32_t MAX_SIZE = 10 * 1024 * 1024;
    if (LoadFile(filename, MAX_SIZE, stderr, &content))
    {
//        std::cout.write(content, size);
//        DebugLexer(content);
        DebugParser(content);

        delete content;
    }
    else
    {
        return 1;
    }

    // execute command https://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c-using-posix

    return 0;
}