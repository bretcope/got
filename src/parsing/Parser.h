#ifndef MOT_PARSER_H
#define MOT_PARSER_H


#include <cstdio>
#include "../io/FileContent.h"
#include "Nodes.h"

namespace mot
{
    /**
     * Parses a configuration file into an abstract syntax tree (AST). Returns true if successful.
     * @param content The content of the file to be parsed.
     * @param errStream The file stream where to write error messages to (e.g. stderr).
     * @param [out] out_tree The root node of the AST, if successful.
     * @return True if the file was parsed successfully.
     */
    bool ParseConfigurationFile(FileContent* content, FILE* errStream, FileNode** out_tree);
}

#endif //MOT_PARSER_H
