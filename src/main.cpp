#include <iostream>
#include <fstream>
#include <locale>
#include "FileReader.h"
#include "Lexer.h"

void DebugLexer(const char* content, uint32_t size)
{
    auto lexer = new Lexer(content, size);

    while (auto token = lexer->Advance())
    {
        token->DebugPrint(stdout, content, false, true);
        delete token;
    }

    delete lexer;
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

    const char* filename = "sample_profile";

    char* content;
    uint32_t size;
    const uint32_t MAX_SIZE = 10 * 1024 * 1024;
    if (LoadFile(filename, MAX_SIZE, stderr, &content, &size))
    {
//        std::cout.write(content, size);
        DebugLexer(content, size);
    }
    else
    {
        return 1;
    }

    // execute command https://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c-using-posix

    return 0;
}