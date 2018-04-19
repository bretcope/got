#ifndef MOT_PARSER_H
#define MOT_PARSER_H


#include <cstdio>
#include "../io/FileContent.h"
#include "Nodes.h"
#include "../io/Console.h"

namespace mot
{
    /**
     * Parses a configuration file into an abstract syntax tree (AST). Returns a FileNode if successful.
     * @param console The console where to write error messages.
     * @param content The content of the file to be parsed.
     */
    UP<FileNode> ParseConfigurationFile(const Console& console, FileContent& content);
}

#endif //MOT_PARSER_H
