#ifndef MOT_PARSER_H
#define MOT_PARSER_H


#include <cstdio>
#include "../io/FileContent.h"
#include "Nodes.h"
#include "../io/Console.h"

namespace mot
{
    /**
     * Parses a configuration file into an abstract syntax tree (AST). Returns true if successful.
     * @param console The console where to write error messages.
     * @param content The content of the file to be parsed.
     * @param [out] out_tree The root node of the AST, if successful.
     * @return True if the file was parsed successfully.
     */
    bool ParseConfigurationFile(const Console& console, FileContent* content, FileNode** out_tree);
}

#endif //MOT_PARSER_H
